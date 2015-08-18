/**
 * @file types.hpp
 * @author Herbert Jones
 * @brief Api types
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include "boost/optional/optional.hpp"

namespace mf
{
namespace api
{

/** The intended target HTTP method. */
enum class RequestMethod
{
    /** The request method is "GET" */
    Get,
    /** The request method is "POST" */
    Post
};

/** Option to prevent SessionMaintainer from starting API HttpRequest.
 * Used if additional options must be set on HttpRequest object. */
enum class RequestStarted
{
    /** Do not start the HttpRequest immediately. */
    No,
    /** Start the HttpRequest before passing back to caller. */
    Yes,
};

struct SessionTokenData
{
    std::string session_token;
    boost::optional<std::string> pkey;
    std::string time;
    int secret_key;
};

enum class ActionResult
{
    Success,
    Failure
};

enum class FilesFoldersOrBoth
{
    Folders,
    Files,
    Both
};

}  // namespace api
}  // namespace mf
