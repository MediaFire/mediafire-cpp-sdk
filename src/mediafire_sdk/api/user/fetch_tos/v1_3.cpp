/**
 * @file fetch_tos.cpp
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

namespace v1_3 = mf::api::user::fetch_tos::v1_3;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "user" */
namespace user {
namespace fetch_tos {
namespace v1_3 {

const std::string api_path("/api/1.3/user/fetch_tos");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl();

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
            std::map<std::string, std::string> * /* query_parts */
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
    response_data_ptr->user_accepted_terms = TOSAccepted::No;

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.terms_of_service.revision",
            &response_data_ptr->revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.terms_of_service.revision\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.terms_of_service.terms",
            &response_data_ptr->terms ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.terms_of_service.terms\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.terms_of_service.date",
            &response_data_ptr->date ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.terms_of_service.date\"");

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.terms_of_service.user_accepted_terms",
                &optval) )
        {
            if ( optval == "no" )
                response_data_ptr->user_accepted_terms = TOSAccepted::No;
            else if ( optval == "yes" )
                response_data_ptr->user_accepted_terms = TOSAccepted::Yes;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.terms_of_service.acceptance_token",
            &response_data_ptr->acceptance_token ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.terms_of_service.acceptance_token\"");

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data); 

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;


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

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_3
}  // namespace fetch_tos
}  // namespace user
}  // namespace api
}  // namespace mf
