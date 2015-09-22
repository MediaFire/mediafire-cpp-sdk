/**
 * @file mediafire_sdk/downloader/detail/header_utils.hpp
 * @author Herbert Jones
 * @brief Functions for handling headers.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <string>

#include "boost/optional/optional.hpp"

#include "mediafire_sdk/http/headers.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

boost::optional<std::string> FilenameFromHeaders(
        const mf::http::Headers & header_container);

}  // namespace detail
}  // namespace downloader
}  // namespace mf
