/**
 * @file timed_events.hpp
 * @author Herbert Jones
 * @brief A class for encapsulating a series of timed actions effeciently.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <memory>

#include "boost/asio.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "mediafire_sdk/utils/mutex.hpp"

namespace mf {
namespace utils {

/**
 * @class TimedEvents
 * @brief A timer wrapper for when multiple items of same type need to be
 *        handled at individual timeouts.
 *
 * This class uses a single asio timer for multiple items instead of the naive
 * approach of having one timer for each timeout.
 *
 * @warning When cancelling, or when TimedEvents is destroyed, all unfired events
 * will fire with std::errc::operation_aborted.  The events will still be
 * processed by the io_service passed to Create.  If calling cancel on your
 * handler's destruction, if you are using a pointer to the handler's object,
 * the pointer will now be invalid.  Be sure to check for this or use a shared
 * pointer to the handler.
 */
template<typename Event>
class TimedEvents :
    public std::enable_shared_from_this<TimedEvents<Event>>
{
public:
    using EventProcessor = std::function<void(Event, std::error_code)>;
    using Pointer = std::shared_ptr<TimedEvents>;
    using Lock = mf::utils::lock_guard<mf::utils::mutex>;
    using UniqueLock = mf::utils::unique_lock<mf::utils::mutex>;
    using TimePoint = std::chrono::steady_clock::time_point;

    /**
     * @param[in] io_service IO service used to execute acions.
     * @param[in] callback Function callback where actions are passed.
     */
    static Pointer Create(
            boost::asio::io_service * io_service,
            EventProcessor event_processor
        )
    {
        return std::shared_ptr<TimedEvents<Event>>( new TimedEvents<Event>(
                io_service, event_processor));
    }

    ~TimedEvents()
    {
        Stop();
    }

    /**
     * @brief Stop performing events so the object can be destroyed
     * automatically once there are no longer any events in the io_service.
     */
    void Stop()
    {
        {
            Lock lock(mutex_);
            running_ = false;
        }
        CancelAll();
    }

    /**
     * @brief Remove all in progress timeouts.
     */
    void CancelAll()
    {
        UniqueLock unique_lock(mutex_);

        timeout_timer_.cancel();

        while (!data_.empty())
        {
            auto event = std::move( data_.begin()->second );
            data_.erase(data_.begin());

#ifdef __MINGW32__
            std::errc errc_error = std::errc::interrupted;
#else
            std::errc errc_error = std::errc::operation_canceled;
#endif
            io_service_->post( boost::bind( event_processor_, std::move(event),
                    std::make_error_code(errc_error)));
        }

        data_.clear();
    }

    /**
     * @brief Add an event to be processed at the specified time.
     *
     * @param[in] duration Interval from now to execute event.
     * @param[in] event Event to execute later.
     */
    void Add(
            std::chrono::milliseconds duration,
            Event event
        )
    {
        const auto now = std::chrono::steady_clock::now();

        Add( now+duration, std::move(event) );
    }

    /**
     * @brief Add an event to be processed at the specified time.
     *
     * @param[in] timeout_time Time for event to execute.
     * @param[in] event Event to execute later.
     */
    void Add(
            TimePoint timeout_time,
            Event event
        )
    {
        Lock lock(mutex_);
        data_.insert( std::make_pair(timeout_time, std::move(event)) );

        io_service_->post(
            boost::bind( &TimedEvents<Event>::ResetTimer,
                this->shared_from_this() ));
    }

    /**
     * @brief Get the number of actions awaiting processing.
     *
     * @return Number of actions yet to be processed.
     */
    std::size_t Size()
    {
        Lock lock(mutex_);
        return data_.size();
    }

private:
    boost::asio::io_service * io_service_;
    boost::asio::steady_timer timeout_timer_;

    bool running_;

    std::multimap<TimePoint, Event> data_;

    EventProcessor event_processor_;

    mf::utils::mutex mutex_;

    /**
     * @param[in] io_service IO service used to execute acions.
     */
    TimedEvents(
            boost::asio::io_service * io_service,
            EventProcessor event_processor
        ) :
        io_service_(io_service),
        timeout_timer_(*io_service),
        running_(true),
        event_processor_(event_processor)
    {
    }

    // Only call on io_service
    void ResetTimer()
    {
        const auto now = std::chrono::steady_clock::now();

        UniqueLock unique_lock(mutex_);

        while (running_ && !data_.empty())
        {
            auto map_front_it = data_.begin();
            const auto timeout = map_front_it->first;
            if ( timeout <= now )
            {
                // Effeciently extract the value so we can move it.
                auto event = std::move( map_front_it->second );
                data_.erase(map_front_it);

                // Unlock incase callback calls a TimedActions method.
                unique_lock.unlock();
                event_processor_(std::move(event), std::error_code());
                unique_lock.lock();
            }
            else
            {
                timeout_timer_.expires_at(timeout);

                timeout_timer_.async_wait(
                    boost::bind(
                        &TimedEvents<Event>::HandleTimeout,
                        this->shared_from_this(),
                        boost::asio::placeholders::error
                    )
                );
                break;
            }
        }
    }

    // Only call on io_service
    void HandleTimeout(
            const boost::system::error_code & err
        )
    {
        if (!err)
        {
            ResetTimer();
        }
    }
};

}  // namespace utils
}  // namespace mf
