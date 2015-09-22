/**
 * @file mediafire_sdk/utils/error_or.hpp
 * @author Herbert Jones
 * @brief Return error or success with value.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <system_error>

#include "boost/optional/optional.hpp"

namespace mf
{
namespace utils
{

/**
 * @class ErrorOr
 * @brief Class for passing extended error information.
 */
template <typename ValueType>
class ErrorOr
{
public:
    /**
     * @brief Successful response constructor.  Copies value.
     *
     * @param[in] value Data to relay.
     *
     * @return ErrorOr
     */
    static ErrorOr Value(const ValueType & value) { return ErrorOr(value); }

    /**
     * @brief Successful response constructor.  Moves value.
     *
     * @param[in] value Data to relay.
     *
     * @return ErrorOr
     */
    static ErrorOr Value(ValueType && value)
    {
        return ErrorOr(std::move(value));
    }

    /**
     * @brief Error response constructor.
     *
     * @param[in] error_code Error code of failure.  Should not be empty.
     * @param[in] extended_description Description of error.
     *
     * @return ErrorOr
     */
    static ErrorOr Error(std::error_code error_code,
                         std::string extended_description)
    {
        assert(error_code);
        return ErrorOr(error_code, extended_description);
    }

    bool IsError() const { return !static_cast<bool>(value_); }
    const std::error_code & GetErrorCode() const { return error_code_; }
    const std::string & GetDescription() const { return description_; }

    bool ContainsValue() const { return static_cast<bool>(value_); }
    const ValueType & GetValue() const { return *value_; }

private:
    ErrorOr(const ValueType & value)
            : error_code_(), description_(), value_(value)
    {
    }

    ErrorOr(ValueType && value)
            : error_code_(), description_(), value_(std::move(value))
    {
    }

    explicit ErrorOr(std::error_code ec, std::string extended_description)
            : error_code_(ec),
              description_(extended_description),
              value_(boost::none)
    {
    }

    std::error_code error_code_;
    std::string description_;
    boost::optional<ValueType> value_;
};

}  // namespace utils
}  // namespace mf
