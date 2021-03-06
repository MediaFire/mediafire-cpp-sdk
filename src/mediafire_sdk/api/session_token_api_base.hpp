/**
 * @file session_token_api_base.hpp
 * @author Herbert Jones
 * @brief Base class for API request types that need session tokens.
 *
 * @copyright Copyright 2014 Mediafire
 *
 * See api_interface.hpp for full example on how to create an API request type.
 */
#pragma once

#include <functional>
#include <map>
#include <string>

#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/api/response_base.hpp"

#include "mediafire_sdk/api/detail/session_token_api_base_static.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace mf {
namespace api {

/**
 * @interface SessionTokenApiBase
 * @brief Templated base class for session token API operations.
 *
 * @tparam ApiResponseType The response datatype for the API request.
 *
 * Contains base functionality for API calls which are custom to each API type.
 */
template<typename ApiResponseType>
class SessionTokenApiBase :
    public detail::SessionTokenApiBaseStatic,
    public detail::ApiBase<ApiResponseType>
{
public:
    /** Requester/SessionMaintainer expected class. */
    typedef ApiResponseType ResponseType;

    /** Requester expected method. */
    std::string Url(const std::string & hostname) const
    {
        std::string path;
        std::map<std::string, std::string> query_parts;

        // We always want JSON response data.
        query_parts["response_format"] = "json";
        // Yet let the implementation override defaults.

        BuildUrl(&path, &query_parts);

        std::string ret = "https://" + hostname +
            MakePathAndQuery(path, query_parts);

#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "URL: " << ret << std::endl;
#       endif
        return ret;
    }

protected:
    /**
     * @brief Pure virtual function so implementer only needs to extract data
     * from already parsed JSON.
     *
     * Classes that wish to use this base must use the property tree in the
     * ResponseType to extract data from the API JSON, or set the error fields in
     * the ResponseType if a failure is detected.
     *
     * @param[in,out] response Pointer to Response with parsed JSON.
     */
    virtual void ParseResponse(ResponseType * response) = 0;

    /**
     * @brief Convenience function for creating a session token based URL.
     *
     * @param[out] path The path of the API URL. Example:
     *             /api/user/get_action_token.php
     * @param[in,out] query_parts Map with url query key pairs. If query
     *                contains "type=upload", then:
     *                query_parts["type"] == "upload"
     */
    virtual void BuildUrl(
            std::string * path,
            std::map<std::string, std::string> * query_parts
        ) const = 0;
};

}  // namespace api
}  // namespace mf
