/**
 * @file src/mediafire_sdk/api/unit_tests/ut_live.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "ut_live.hpp"

#include "boost/random/random_device.hpp"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int_distribution.hpp"
#include "boost/test/unit_test.hpp"

#include "mediafire_sdk/api/device/get_status.hpp"
#include "mediafire_sdk/utils/variant.hpp"

namespace posix_time = boost::posix_time;
namespace api = mf::api;

namespace ut
{

namespace constants
{
const std::string host = "www.mediafire.com";
}  // namespace constants

namespace user1
{
const std::string username = TEST_USER_1_USERNAME;
const std::string password = TEST_USER_1_PASSWORD;
}  // namespace user1

namespace user2
{
const std::string username = TEST_USER_2_USERNAME;
const std::string password = TEST_USER_2_PASSWORD;
}  // namespace user2

namespace globals
{
const api::credentials::Email connection1{user1::username, user1::password};
const api::credentials::Email connection2{user2::username, user2::password};
}  // namespace globals

Fixture::Fixture()
        : credentials_(globals::connection1),
          http_config_(mf::http::HttpConfig::Create()),
          timeout_timer_(*http_config_->GetWorkIoService()),
          call_in_timer_(*http_config_->GetWorkIoService()),
          stm_(http_config_, constants::host),
          async_wait_logged_(false)
{
    stm_.SetSessionStateChangeCallback(
            boost::bind(&Fixture::HandleSessionStateChange, this, _1));
    stm_.SetConnectionStateChangeCallback(
            boost::bind(&Fixture::HandleConnectionStateChange, this, _1));
    stm_.SetLoginCredentials(credentials_);
}

Fixture::~Fixture() { http_config_->StopService(); }

void Fixture::SetUser2() { credentials_ = globals::connection2; }

void Fixture::Start()
{
    assert(http_config_);

    http_config_->GetWorkIoService()->reset();
    http_config_->RunService();
}

void Fixture::StartWithDefaultTimeout()
{
    StartWithTimeout(posix_time::seconds(15));
}

void Fixture::StartWithTimeout(boost::posix_time::time_duration timeout_length)
{
    timeout_timer_.expires_from_now(timeout_length);

    timeout_timer_.async_wait(boost::bind(&Fixture::HandleTimeout, this,
                                          boost::asio::placeholders::error));

    Start();
}

void Fixture::Stop()
{
    timeout_timer_.cancel();

    // Stop any ongoing timeouts so that io_service::run stops on its own
    stm_.StopTimeouts();
}

void Fixture::Fail(const boost::optional<std::string> & maybe_str)
{
    if (maybe_str)
        Fail(*maybe_str);
    else
        Fail(std::string("Unknown error"));
}

void Fixture::Fail(const std::string & str)
{
    Stop();

    std::cout << TestName() << ": Failure" << std::endl;
    BOOST_FAIL(str);
}

void Fixture::Fail(const char * errorString) { Fail(std::string(errorString)); }

void Fixture::Success()
{
    std::cout << TestName() << ": Success" << std::endl;

    WaitForAnyAsyncOperationsToComplete();
}

void Fixture::WaitForAnyAsyncOperationsToComplete()
{
    timeout_timer_.cancel();

    Call(api::device::get_status::Request(),
         [&](const api::device::get_status::Response & response)
         {
             if (!response.response_data)
             {
                 // Ignore error, try again
                 WaitForAnyAsyncOperationsToComplete();
             }
             else
             {
                 const auto & data = *response.response_data;

                 if (data.async_jobs_in_progress
                     == api::device::get_status::AsyncJobs::Stopped)
                 {
                     Stop();
                 }
                 else
                 {
                     if (!async_wait_logged_)
                     {
                         async_wait_logged_ = true;
                         Log("Waiting for asynchronous operation to complete.");
                     }
                     WaitForAnyAsyncOperationsToComplete();
                 }
             }
         });
}

void Fixture::ChangeCredentials(const api::Credentials & credentials)
{
    stm_.SetLoginCredentials(credentials);
}

void Fixture::Debug(const boost::property_tree::wptree & pt)
{
    std::wostringstream ss;
    boost::property_tree::write_json(ss, pt);
    std::cout << TestName() << ": JSON received:\n"
              << mf::utils::wide_to_bytes(ss.str()) << std::endl;
}

void Fixture::Debug(const std::string & str)
{
    std::ostringstream ss;
    std::cout << TestName() << ": " << str << std::endl;
}

void Fixture::HandleTimeout(const boost::system::error_code & err)
{
    if (!err)
    {
        std::ostringstream ss;
        ss << "Timeout failure in " << TestName() << std::endl;
        BOOST_FAIL(ss.str());

        // Stop not safe in this instance. Cancel all requests instead.
        for (const api::SessionMaintainer::Request & request : requests_)
            request->Cancel();
    }
}

void Fixture::HandleConnectionStateChange(
        mf::api::ConnectionState new_connection_state)
{
    mf::utils::Match(new_connection_state,
                     [](const mf::api::connection_state::Uninitialized &)
                     {
                     },
                     [](const mf::api::connection_state::Unconnected & state)
                     {
                         std::cout << "Connection state: Unconnected\n"
                                   << "Reason: " << state.error_code.message()
                                   << std::endl;
                     },
                     [](const mf::api::connection_state::Connected &)
                     {
                     });
}

void Fixture::HandleSessionStateChange(mf::api::SessionState new_session_state)
{
    // boost::apply_visitor(visitor, new_session_state);
    mf::utils::Match(
            new_session_state,
            [](const mf::api::session_state::Uninitialized &)
            {
            },
            [](const mf::api::session_state::Initialized &)
            {
            },
            [](const mf::api::session_state::CredentialsFailure & state)
            {
                std::cout << "Session state: Error\n"
                          << "Reason: " << state.error_code.message()
                          << std::endl;
            },
            [](const mf::api::session_state::ProlongedError & state)
            {
                std::cout << "Session state: ProlongedError"
                          << "Reason: " << state.error_code.message()
                          << std::endl;
            },
            [](const mf::api::session_state::Running &)
            {
            });
}

std::string RandomAlphaNum(int length)
{
    std::string ret;

    // No hex chars here to avoid accidental success if chunking is broken.
    static std::string chars(
            "0123456789"
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    static boost::random::random_device rng;
    static boost::random::uniform_int_distribution<> index_dist(
            0, chars.size() - 1);

    ret.reserve(length);

    for (int i = 0; i < length; ++i)
        ret.push_back(chars[index_dist(rng)]);

    return ret;
}

std::string TestName()
{
    return boost::unit_test::framework::current_test_case().p_name;
}

}  // namespace ut
