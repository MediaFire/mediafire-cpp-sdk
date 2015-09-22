/**
 * @file http_config.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "http_config.hpp"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/system/system_error.hpp"

#include "mediafire_sdk/http/detail/default_http_headers.hpp"
#include "mediafire_sdk/http/detail/default_pem.hpp"

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;

namespace
{
ssl::context DefaultSslContext()
{
    using mf::http::detail::pem;

    // See https://www.openssl.org/docs/manmaster/ssl/SSL_CTX_new.html
    // And boost/asio/ssl/impl/context.ipp

    // sslv23_client: A TLS/SSL connection established with these methods may
    // understand the SSLv2, SSLv3, TLSv1, TLSv1.1 and TLSv1.2 protocols.
    ssl::context ssl_ctx(ssl::context::sslv23_client);

    // However we want to disable SSL and leave TLS
    ssl_ctx.set_options(ssl::context::default_workarounds
                        | ssl::context::no_sslv2 | ssl::context::no_sslv3);

    // Use Operating System's certificates if available
    ssl_ctx.set_default_verify_paths();

    for (std::size_t i = 0; i < (sizeof(pem) / sizeof(pem[0])); ++i)
    {
        try
        {
            // This can fail if the certificate exists already due to the call
            // to set_default_verify_paths.
            ssl_ctx.add_certificate_authority(
                    asio::const_buffer(pem[i], std::strlen(pem[i])));
        }
        catch (const boost::system::system_error & ex)
        {
            // We should be able to safely ignore the error.
        }
    }

    return ssl_ctx;
}
mf::http::HeaderContainer DefaultHeaders()
{
    mf::http::HeaderContainer default_headers;

    for (std::size_t i = 0;
        i < ( sizeof(mf::http::default_headers)
            / sizeof(mf::http::default_headers[0]) );
        ++i
    )
    {
        default_headers.emplace_back(
            mf::http::default_headers[i][0],
            mf::http::default_headers[i][1] );
    }

    return default_headers;
}
}  // namespace

namespace mf {
namespace http {

HttpConfig::Pointer HttpConfig::Create()
{
    return std::shared_ptr<HttpConfig>(new HttpConfig);
}

HttpConfig::Pointer HttpConfig::Create(
        boost::asio::io_service * io_service
    )
{
    auto http_config = std::shared_ptr<HttpConfig>(new HttpConfig);
    http_config->SetWorkIoService(io_service);
    return http_config;
}

HttpConfig::Pointer HttpConfig::Create(
        boost::asio::io_service * io_service,
        boost::asio::io_service * callback_io_service
    )
{
    auto http_config = std::shared_ptr<HttpConfig>(new HttpConfig);
    http_config->SetWorkIoService(io_service);
    http_config->SetDefaultCallbackIoService(callback_io_service);
    return http_config;
}

HttpConfig::Pointer HttpConfig::Clone() const
{
    Pointer new_ptr = std::shared_ptr<HttpConfig>(new HttpConfig(*this));
    return new_ptr;
}

HttpConfig::HttpConfig() :
    io_service_(nullptr),
    default_callback_io_service_(nullptr),
    self_signed_certs_allowed_(SelfSigned::Denied),
    redirect_policy_(RedirectPolicy::Allow),
    default_headers_(DefaultHeaders()),
    bandwidth_usage_percent_(100)
{
}

HttpConfig::~HttpConfig()
{
}

void HttpConfig::SetWorkIoService(boost::asio::io_service * ios)
{
    assert(ios != nullptr);

    if (owned_io_service_)
        owned_io_service_.reset();

    io_service_ = ios;
}

boost::asio::io_service * HttpConfig::GetWorkIoService() const
{
    if (!io_service_)
    {
        owned_io_service_ = std::make_shared<boost::asio::io_service>();
        io_service_ = owned_io_service_.get();
    }

    return io_service_;
}

void HttpConfig::RunService()
{
    boost::asio::io_service * io_service = GetWorkIoService();
    io_service->run();
}

void HttpConfig::StopService()
{
    boost::asio::io_service * io_service = GetWorkIoService();
    io_service->stop();
}

void HttpConfig::SetDefaultCallbackIoService(boost::asio::io_service * ios)
{
    assert(ios != nullptr);

    default_callback_io_service_ = ios;
}

boost::asio::io_service * HttpConfig::GetDefaultCallbackIoService() const
{
    if ( default_callback_io_service_ != nullptr )
        return default_callback_io_service_;
    else
        return GetWorkIoService();
}

void HttpConfig::SetBandwidthAnalyser(BandwidthAnalyserInterface::Pointer bwa)
{
    bandwidth_analyser_ = bwa;
}

BandwidthAnalyserInterface::Pointer HttpConfig::GetBandwidthAnalyser() const
{
    return bandwidth_analyser_;
}

ssl::context * HttpConfig::GetSslContext() const
{
    if (!ssl_ctx_)
    {
        ssl_ctx_ = std::make_shared<ssl::context>(DefaultSslContext());
    }

    return ssl_ctx_.get();
}

void HttpConfig::SetSslContext(std::shared_ptr<boost::asio::ssl::context> ctx)
{
    assert(ctx.get() != nullptr);

    ssl_ctx_ = ctx;
}

void HttpConfig::AddDefaultHeader(
        std::string key,
        std::string value
    )
{
    for (auto & pair : default_headers_)
    {
        if (boost::iequals(pair.first, key))
        {
            pair.second = value;
            return;
        }
    }
    default_headers_.emplace_back(key, value);
}

}  // namespace http
}  // namespace mf
