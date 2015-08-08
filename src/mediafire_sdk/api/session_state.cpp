/**
 * @file session_state.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "session_state.hpp"

#include "mediafire_sdk/utils/variant.hpp"

using mf::utils::Match;

namespace mf
{
namespace api
{
namespace session_state
{

bool operator==(const Uninitialized & /*lhs*/, const Uninitialized & /*rhs*/)
{
    return true;
}

bool operator==(const Initialized & /*lhs*/, const Initialized & /*rhs*/)
{
    return true;
}

bool operator==(const CredentialsFailure & lhs, const CredentialsFailure & rhs)
{
    // Only the parts we care about determine if the session token response is
    // different than another.

    return std::tie(lhs.pkey, lhs.error_code)
           == std::tie(rhs.pkey, rhs.error_code);
}

bool operator==(const ProlongedError & lhs, const ProlongedError & rhs)
{
    // Only the parts we care about determine if the session token response is
    // different than another.

    return std::tie(lhs.error_code) == std::tie(rhs.error_code);
}

bool operator==(const Running & lhs, const Running & rhs)
{
    const api::user::get_session_token::Response & l
            = lhs.session_token_response;

    const api::user::get_session_token::Response & r
            = rhs.session_token_response;

    if (!l.response_data && !r.response_data)
    {
        return true;
    }
    else if (!l.response_data)
    {
        return false;
    }
    else if (!r.response_data)
    {
        return false;
    }
    else
    {
        // Only the parts we care about determine if the session token response
        // is different than another.

        return std::tie(l.response_data->session_token, l.pkey)
               == std::tie(r.response_data->session_token, r.pkey);
    }
}

}  // namespace session_state

std::ostream & operator<<(std::ostream & out,
                          const mf::api::SessionState & state)
{
    out << Match(state,
                 [](const session_state::Uninitialized &) -> std::string
                 {
                     return "Uninitialized{}";
                 },
                 [](const session_state::Initialized &) -> std::string
                 {
                     return "Initialized{}";
                 },
                 [](const session_state::CredentialsFailure & state)
                         -> std::string
                 {
                     std::ostringstream ss;
                     ss << "CredentialsFailure{";
                     if (state.pkey)
                         ss << *state.pkey;
                     else
                         ss << "no pkey";

                     ss << ", " << state.error_code.message() << ", "
                        << state.session_token_response.error_string << "}";

                     return ss.str();
                 },
                 [](const session_state::ProlongedError & state) -> std::string
                 {
                     std::ostringstream ss;
                     ss << "ProlongedError{";
                     ss << state.error_code.message() << ", "
                        << state.session_token_response.error_string;
                     ss << "}";

                     return ss.str();
                 },
                 [](const session_state::Running &) -> std::string
                 {
                     return "Running{}";
                 });

    return out;
}

}  // namespace api
}  // namespace mf
