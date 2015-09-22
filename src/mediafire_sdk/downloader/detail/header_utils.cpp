/**
 * @file mediafire_sdk/downloader/detail/header_utils.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "header_utils.hpp"

namespace mf
{
namespace downloader
{
namespace detail
{

boost::optional<std::string> FilenameFromHeaders(
        const mf::http::Headers & header_container)
{
    const auto & header_map = header_container.headers;

    auto content_disposition_it = header_map.find("content-disposition");
    if (content_disposition_it != header_map.end())
    {
        std::string id = "filename=";
        const auto & search_area = content_disposition_it->second;
        const auto pos = search_area.find(id);
        if (pos != search_area.npos)
        {
            const auto start_pos = pos + id.size() + 1;
            const auto end_pos = search_area.size();
            if (pos + id.size() < end_pos)
            {
                const auto delimiter = search_area[pos + id.size()];
                for (auto check_pos = start_pos; check_pos < end_pos;
                     ++check_pos)
                {
                    const auto check_ch = search_area[check_pos];
                    if (check_ch == '\\')
                    {
                        ++check_pos;
                    }
                    else if (check_ch == delimiter)
                    {
                        return search_area.substr(start_pos,
                                                  check_pos - start_pos);
                    }
                }
            }
        }
    }

    return boost::none;
}

}  // namespace detail
}  // namespace downloader
}  // namespace mf
