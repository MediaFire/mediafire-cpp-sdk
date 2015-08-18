/** @file .get_login_tokencpp @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "get_login_token.hpp"

#include <string>

#include "boost/property_tree/json_parser.hpp"
#include "boost/variant/apply_visitor.hpp"

#include "mediafire_sdk/api/app_constants.hpp"
#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/url_encode.hpp"

// #define OUTPUT_DEBUG

namespace v0 = mf::api::user::get_login_token::v0;

namespace
{
std::string AssembleQueryParts(std::map<std::string, std::string> & parts)
{
    std::string query;
    char divider = '?';
    for (const auto & it : parts)
    {
        query += divider + it.first + '='
                 + mf::utils::url::get_parameter::Encode(it.second);
        divider = '&';
    }
    return query;
}
class CredentialsParts
        : public boost::static_visitor<std::map<std::string, std::string>>
{
public:
    std::map<std::string, std::string> operator()(
            const mf::api::credentials::Email & email_credentials) const
    {
        std::map<std::string, std::string> parts;

        parts.emplace(std::string("email"), email_credentials.email);
        parts.emplace(std::string("password"), email_credentials.password);
        parts.emplace(
                std::string("signature"),
                app_constants::BuildSignature(email_credentials.email
                                              + email_credentials.password));

        return parts;
    }

    std::map<std::string, std::string> operator()(
            const mf::api::credentials::Ekey & ekey_credentials) const
    {
        std::map<std::string, std::string> parts;

        parts.emplace(std::string("ekey"), ekey_credentials.ekey);
        parts.emplace(std::string("password"), ekey_credentials.password);
        parts.emplace(
                std::string("signature"),
                app_constants::BuildSignature(ekey_credentials.ekey
                                              + ekey_credentials.password));

        return parts;
    }

    std::map<std::string, std::string> operator()(
            const mf::api::credentials::Facebook & facebook_credentials) const
    {
        std::map<std::string, std::string> parts;

        parts.emplace(std::string("fb_access_token"),
                      facebook_credentials.fb_access_token);
        parts.emplace(std::string("signature"),
                      app_constants::BuildSignature(
                              facebook_credentials.fb_access_token));

        return parts;
    }
};
}  // namespace

mf::api::user::get_login_token::v0::Request::Request(
        const Credentials & credentials)
        : credentials_(credentials)
{
}

std::string v0::Request::Url(std::string hostname) const
{
    std::map<std::string, std::string> query_parts;

    static const bool has_app_id = (app_constants::kAppId != nullptr
                                    && std::strlen(app_constants::kAppId) > 0);

    if (has_app_id)
        query_parts.emplace(std::string("application_id"),
                            app_constants::kAppId);
    else
        assert(!"app_constants::kAppId not defined!");

    query_parts.emplace(std::string("response_format"), std::string("json"));

    const std::map<std::string, std::string> credential_parts
            = boost::apply_visitor(CredentialsParts(), credentials_);
    query_parts.insert(credential_parts.begin(), credential_parts.end());

    std::string url;
    url = "https://" + hostname + "/api/user/get_login_token.php";
    url += AssembleQueryParts(query_parts);
    return url;
}

void v0::Request::HandleContent(const std::string & url,
                                const mf::http::Headers & headers,
                                const std::string & content)
{
    assert(callback_);
    if (!callback_)
        return;

    ResponseType response;
    response.InitializeWithContent(url, "", headers, content);

    ResponseData response_data;

#ifdef OUTPUT_DEBUG  // Debug code
    std::cout << "Got content:\n" << content << std::endl;

    std::wostringstream ss;
    boost::property_tree::write_json(ss, response.pt);
    std::cout << "Got JSON:\n" << mf::utils::wide_to_bytes(ss.str())
              << std::endl;
#endif

    {
        std::string pkey;
        if (GetIfExists(response.pt, "response.pkey", &pkey))
            response.pkey = pkey;
    }

    if (!response.error_code)
    {
        if (!GetIfExists(response.pt, "response.login_token",
                         &response_data.login_token))
        {
            response.error_code
                    = make_error_code(mf::api::api_code::ContentInvalidData);
            response.error_string = "missing session token";
        }
    }

    // Only return data structure if no errors
    if (!response.error_code)
        response.response_data = std::move(response_data);

    callback_(response);
}

void v0::Request::HandleError(const std::string & url,
                              std::error_code ec,
                              const std::string & error_string)
{
    if (!callback_)
        return;

    ResponseType response;
    response.InitializeWithError(url, "", ec, error_string);
    callback_(response);
}
