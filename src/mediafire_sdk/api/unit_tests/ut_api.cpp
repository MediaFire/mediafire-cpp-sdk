/*
 * @copyright Copyright 2014 Mediafire
 */
#define WIN32_LEAN_AND_MEAN

#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/http/post_data_pipe_interface.hpp"
#include "mediafire_sdk/http/error.hpp"

#include "mediafire_sdk/api/requester.hpp"
#include "mediafire_sdk/api/user/get_session_token.hpp"
#include "mediafire_sdk/api/folder/get_content.hpp"

#include "mediafire_sdk/api/unit_tests/api_ut_helpers.hpp"

#include "mediafire_sdk/utils/base64.hpp"
#include "mediafire_sdk/utils/string.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE Requester
#include "boost/test/unit_test.hpp"

namespace asio = boost::asio;
namespace api = mf::api;

#if !defined(TEST_USER_1_USERNAME) || !defined(TEST_USER_1_PASSWORD)
#error "TEST_USER defines not set."
#endif

const std::string username = TEST_USER_1_USERNAME;
const std::string password = TEST_USER_1_PASSWORD;

BOOST_AUTO_TEST_CASE(SessionTokenSuccess)
{
    std::string session_token = "1373dfdff67e233e5303141afc47ec1b69dd4105f24c23"
        "388e95260b26c834f0436929357e3ba5b9b0468f0d8a1a2de3155688b3aa1f8eb55342"
        "49abbf003e3839e9035f2275eb10";

    boost::property_tree::wptree pt
            = api::ut::MakeBaseApiContent("user/get_session_token");
    pt.put(L"response.session_token", mf::utils::bytes_to_wide(session_token));
    pt.put(L"response.secret_key", L"1955614760");
    pt.put(L"response.time", L"1396873639.6026");
    pt.put(L"response.pkey", L"09b3b02524");
    pt.put(L"response.ekey", L"9642583c5f69d94c45eff31123a5bc01");
    pt.put(L"response.current_api_version", L"1.1");

    std::string request_header_regex = "GET.*get_session_token.*\r\n\r\n";

    typedef api::ut::ApiExpectServer<api::user::get_session_token::Request>
            Wrapper;
    Wrapper wrapper(pt, request_header_regex, 200, "OK");

    api::user::get_session_token::Request request(
            api::credentials::Email{username, password});

    wrapper.Run(request, boost::bind(&Wrapper::Callback, &wrapper, _1));

    auto & response = wrapper.Data();

    BOOST_CHECK(!response.error_code);

    if (response.error_code)
    {
        std::cout << "Error code: " << response.error_code << std::endl;

        if (response.error_string)
            std::cout << "Error code: " << response.error_string << std::endl;
    }
    else
    {
        const auto & response_data = *response.response_data;

        BOOST_CHECK_EQUAL(response_data.session_token, session_token);
    }
}

BOOST_AUTO_TEST_CASE(SessionTokenFailure)
{
    boost::property_tree::wptree pt = api::ut::MakeErrorApiContent(
            "user/get_session_token", "The Credentials you entered are invalid",
            107);

    std::string request_header_regex = "GET.*get_session_token.*\r\n\r\n";

    typedef api::ut::ApiExpectServer<api::user::get_session_token::Request>
            Wrapper;
    Wrapper wrapper(pt, request_header_regex, 403, "Forbidden");

    api::user::get_session_token::Request request(
            api::credentials::Email{username, password});

    wrapper.Run(request, boost::bind(&Wrapper::Callback, &wrapper, _1));

    BOOST_CHECK(wrapper.Data().error_code);
}
