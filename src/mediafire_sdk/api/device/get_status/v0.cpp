/**
 * @file get_status.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "v0.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v0 = mf::api::device::get_status::v0;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "device" */
namespace device {
namespace get_status {
namespace v0 {

const std::string api_path("/api/device/get_status");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl();

    boost::optional<uint32_t> device_id_;
    virtual void BuildUrl(
        std::string * path,
        std::map<std::string, std::string> * query_parts
    ) const override;

    virtual void ParseResponse( Response * response ) override;

    mf::http::SharedBuffer::Pointer GetPostData();

    mf::api::RequestMethod GetRequestMethod() const
    {
        return mf::api::RequestMethod::Post;
    }
};

Impl::Impl()
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

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.async_jobs_in_progress",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->async_jobs_in_progress = AsyncJobs::Stopped;
            else if ( optval == "yes" )
                response_data_ptr->async_jobs_in_progress = AsyncJobs::Running;
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in response.async_jobs_in_progress");
        }
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in response.async_jobs_in_progress");
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.device_revision",
            &response_data_ptr->device_revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.device_revision\"");

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data);

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    if (device_id_)
        parts["device_id"] = AsString(*device_id_);

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request() :
    impl_(new Impl())
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

void Request::SetDeviceId(uint32_t device_id)
{
    impl_->device_id_ = device_id;
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v0
}  // namespace get_status
}  // namespace device
}  // namespace api
}  // namespace mf
