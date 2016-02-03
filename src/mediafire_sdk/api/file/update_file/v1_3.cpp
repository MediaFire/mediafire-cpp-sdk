/**
 * @file update_file.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "v1_3.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v1_3 = mf::api::file::update_file::v1_3;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "file" */
namespace file {
namespace update_file {
namespace v1_3 {

const std::string api_path("/api/1.3/file/update_file");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl(
            std::string from_quickkey,
            std::string to_quickkey
        );

    std::string from_quickkey_;
    std::string to_quickkey_;
    virtual void BuildUrl(
        std::string * path,
        std::map<std::string, std::string> * query_parts
    ) const override;

    virtual void ParseResponse( Response * response ) override;

    mf::http::SharedBuffer::Pointer GetPostData();

    mf::api::RequestMethod GetRequestMethod() const override
    {
        return mf::api::RequestMethod::Post;
    }
};

Impl::Impl(
        std::string from_quickkey,
        std::string to_quickkey
    ) :
    from_quickkey_(from_quickkey),
    to_quickkey_(to_quickkey)
{
}

void Impl::BuildUrl(
            std::string * path,
            std::map<std::string, std::string> * query_parts
    ) const
{
    *path = api_path + ".php";
}

void Impl::ParseResponse( Response * response )
{
    // This function uses return defines for readability and maintainability.
#   define return_error(error_type, error_message)                             \
    {                                                                          \
        SetError(response, error_type, error_message);                         \
        return;                                                                \
    }

    ResponseData response_data;

    // For uniformity for code generation with the other content parsers.
    ResponseData * response_data_ptr = &response_data;

    // create_content_parse_single optional no default
    {
        uint32_t optarg;
        if ( GetIfExists(
                response->pt,
                "response.new_device_revision",
                &optarg) )
        {
            response_data_ptr->new_device_revision = optarg;
        }
    }

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["from_quickkey"] = from_quickkey_;
    parts["to_quickkey"] = to_quickkey_;

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string from_quickkey,
        std::string to_quickkey
    ) :
    impl_(new Impl(from_quickkey, to_quickkey))
{
}

void Request::SetCallback( CallbackType callback_function )
{
    impl_->SetCallback(callback_function);
}

void Request::HandleContent(
        const std::string & url,
        const mf::http::Headers & headers,
        const std::string & content
    )
{
    impl_->HandleContent(url, headers, content);
}

void Request::HandleError(
        const std::string & url,
        std::error_code ec,
        const std::string & error_string
    )
{
    impl_->HandleError(url, ec, error_string);
}

std::string Request::Url(const std::string & hostname) const
{
    return impl_->Url(hostname);
}

void Request::SetSessionToken(
        std::string session_token,
        std::string time,
        int secret_key
    )
{
    impl_->SetSessionToken(session_token, time, secret_key);
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_3
}  // namespace update_file
}  // namespace file
}  // namespace api
}  // namespace mf
