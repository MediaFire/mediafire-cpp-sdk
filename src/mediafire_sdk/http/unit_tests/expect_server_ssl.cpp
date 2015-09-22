/**
 * @file expect_server_ssl.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "expect_server_ssl.hpp"

using boost::logic::indeterminate;
using boost::asio::ip::tcp;
namespace asio = boost::asio;

namespace {
    char const  * const kPemCertificate =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBAMi5assnE7RG2iYF\n"
        "NIOqA7lbVAH3g/NN3bflghKFZCUtKe3qlS3dgDujs4EvPA1yGrV4EUOs0XvFYebC\n"
        "x+1BbsWAmQZnHQSHztAt6+DTCywJQJJUlbKwqsaanrnJ/dvARTEckXYKLRcgmfjf\n"
        "fYqXyDNQxiV6pPa2KsVMwoGBUxLNAgMBAAECgYAVajLKmdCwWx7LD6MaqPjcvbGo\n"
        "xA9/1b7h78qAz1pv3PGsQGrKCee0dTKhhbiSdqoC0lyFK9rtqZFYXU+XMHPwzfdx\n"
        "IIOq/lagyzgEqTvHA0GEK1lzO8Eegsf5y67FWZXNlRpKmFPCmFFyEpOUKHK0x6HN\n"
        "61/iZjMTXRo/AvFcAQJBAOY7BPjC7rphMqcLtw6Bb+WRId90VF72h3Q/LqLxtT2E\n"
        "Vv+n/+Ecl0C9UjhAQ/+wCHCkE/M5/VmS80fKzNUXDVECQQDfMO5kzaeYpUyd/Elb\n"
        "E2RNjTC4AAdtqel6fiGShfYrDxajXEq25CnnVFyxk7q6/ehM2mVnR1IuswDgvBhd\n"
        "It69AkEAkCiseEc2zCVIXiiLut15fzldCFoC6mNbdYKKZSUL4zUWdIZxRjdszfC9\n"
        "ptM2wMcswbs7crUA2jGVe4KUt2jzwQJARUT2eCqrvWBwKwhF7BJUqw0K9dBsfcii\n"
        "QfYrjUIuaKbCK+lU9vZRWw5/xk1HQwnSsyeFGUy1YPEFcLpwBVfxMQJAEj4TJ4rI\n"
        "T7lwtPOSek4nsVawxR95aAaqL1Qt+Khti9qAP6WM3OU7coRnfLjYLyPjpHnbFgZP\n"
        "7vv+EceHXehx8w==\n"
        "-----END PRIVATE KEY-----\n"
        "-----BEGIN CERTIFICATE-----\n"
        "MIIC7jCCAlegAwIBAgIJAPSjVyLAs5aJMA0GCSqGSIb3DQEBBQUAMIGPMQswCQYD\n"
        "VQQGEwJVUzEOMAwGA1UECAwFVGV4YXMxEDAOBgNVBAcMB0hvdXN0b24xEjAQBgNV\n"
        "BAoMCU1lZGlhRmlyZTEQMA4GA1UECwwHRGVza3RvcDESMBAGA1UEAwwJTWVkaWFG\n"
        "aXJlMSQwIgYJKoZIhvcNAQkBFhVoZXJiZXJ0QG1lZGlhZmlyZS5jb20wHhcNMTQw\n"
        "MzMxMjExNTUyWhcNMTQwNDMwMjExNTUyWjCBjzELMAkGA1UEBhMCVVMxDjAMBgNV\n"
        "BAgMBVRleGFzMRAwDgYDVQQHDAdIb3VzdG9uMRIwEAYDVQQKDAlNZWRpYUZpcmUx\n"
        "EDAOBgNVBAsMB0Rlc2t0b3AxEjAQBgNVBAMMCU1lZGlhRmlyZTEkMCIGCSqGSIb3\n"
        "DQEJARYVaGVyYmVydEBtZWRpYWZpcmUuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GN\n"
        "ADCBiQKBgQDIuWrLJxO0RtomBTSDqgO5W1QB94PzTd235YIShWQlLSnt6pUt3YA7\n"
        "o7OBLzwNchq1eBFDrNF7xWHmwsftQW7FgJkGZx0Eh87QLevg0wssCUCSVJWysKrG\n"
        "mp65yf3bwEUxHJF2Ci0XIJn4332Kl8gzUMYleqT2tirFTMKBgVMSzQIDAQABo1Aw\n"
        "TjAdBgNVHQ4EFgQULKVRo3R7fYQgUMjH5P4WR4nXqFswHwYDVR0jBBgwFoAULKVR\n"
        "o3R7fYQgUMjH5P4WR4nXqFswDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOB\n"
        "gQDGbN4rijD2U08t3CmGyQQwpq3jd2hHY5XggBr6ir9hHSZIFxq0zf4eCATRhKKQ\n"
        "sYq3HokxmzzjOS/7Cs+Y0w70URgStkYale5Cc0oAIAABJmSrR4jOH3WXyOFz7Zrm\n"
        "2mv1n5e34d6/1dsmWOFQaQgtKahMrabqLAtH4eVpBO//aQ==\n"
        "-----END CERTIFICATE-----\n";

}  // namespace

ExpectServerSsl::Pointer ExpectServerSsl::Create(
        asio::io_service * io_service,
        std::shared_ptr<asio::io_service::work> work,
        uint16_t port
    )
{
    std::unique_ptr<boost::asio::ssl::context> ssl_ctx(
        new boost::asio::ssl::context(
            boost::asio::ssl::context::sslv23_server ) );

    ssl_ctx->set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        // | boost::asio::ssl::context::single_dh_use
        );

    ssl_ctx->use_certificate_chain(
            asio::const_buffer(kPemCertificate, strlen(kPemCertificate)) );

    ssl_ctx->use_private_key(
            asio::const_buffer(kPemCertificate, strlen(kPemCertificate)),
            boost::asio::ssl::context::pem );

    std::shared_ptr<ExpectServerSsl> ptr(
            new ExpectServerSsl(
                io_service,
                std::move(ssl_ctx),
                work,
                port
                )
            );

    ptr->CreateInit();

    return ptr;
}

ExpectServerSsl::ExpectServerSsl(
        asio::io_service * io_service,
        std::unique_ptr<boost::asio::ssl::context> ssl_ctx,
        std::shared_ptr<asio::io_service::work> work,
        uint16_t port
        ) :
    ExpectServerBase(io_service, work, port),
    acceptor_(*io_service, tcp::endpoint(tcp::v4(), port_)),
    ssl_ctx_(std::move(ssl_ctx)),
    ssl_socket_(*io_service, *(ssl_ctx_.get())),
    ssl_started_(false)
{
}

void ExpectServerSsl::CloseSocket() { ssl_socket_.lowest_layer().close(); }

void ExpectServerSsl::AsyncAccept()
{
    std::cout << "Calling async_accept" << std::endl;

    acceptor_.async_accept(ssl_socket_.lowest_layer(),
            boost::bind(
                &ExpectServerSsl::HandleAccept,
                shared_from_this(),
                asio::placeholders::error
                ));
}

void ExpectServerSsl::SendMessageWrite(
        const expect_server_test::SendMessage & node
    )
{
    std::cout << "Calling async_write" << std::endl;

    if ( ! ssl_started_ )
    {
        asio::async_write(
                ssl_socket_.next_layer(),
                asio::buffer(*node.message),
                boost::bind(
                    &ExpectServerSsl::HandleWrite,
                    shared_from_this(),
                    node.message,  // Pass to keep memory alive
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error
                    )
                );
    }
    else
    {
        asio::async_write(
                ssl_socket_,
                asio::buffer(*node.message),
                boost::bind(
                    &ExpectServerSsl::HandleWrite,
                    shared_from_this(),
                    node.message,  // Pass to keep memory alive
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error
                    )
                );
    }
}

void ExpectServerSsl::ExpectRegexRead(const ExpectRegex & node)
{
    std::cout << "Calling async_read_until" << std::endl;

    if ( ! ssl_started_ )
    {
        asio::async_read_until(
                ssl_socket_.next_layer(),
                read_buffer_,
                node.regex,
                boost::bind(
                    &ExpectServerSsl::HandleReadRegex,
                    shared_from_this(),
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error
                    )
                );
    }
    else
    {
        asio::async_read_until(
                ssl_socket_,
                read_buffer_,
                node.regex,
                boost::bind(
                    &ExpectServerSsl::HandleReadRegex,
                    shared_from_this(),
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error
                    )
                );
    }
}

void ExpectServerSsl::ExpectContentLengthRead(
        uint64_t bytes_to_read,
        const std::size_t total_bytes,
        std::size_t total_read_bytes
    )
{
    std::cout << "Calling async_read" << std::endl;

    if ( ! ssl_started_ )
    {
        asio::async_read(
                ssl_socket_.next_layer(),
                read_buffer_,
                asio::transfer_exactly(static_cast<std::size_t>(bytes_to_read)),
                boost::bind(
                    &ExpectServerSsl::HandleReadContent,
                    shared_from_this(),
                    total_bytes,
                    total_read_bytes,
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error
                    )
                );
    }
    else
    {
        asio::async_read(
                ssl_socket_,
                read_buffer_,
                asio::transfer_exactly(static_cast<std::size_t>(bytes_to_read)),
                boost::bind(
                    &ExpectServerSsl::HandleReadContent,
                    shared_from_this(),
                    total_bytes,
                    total_read_bytes,
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error
                    )
                );
    }
}

void ExpectServerSsl::Handshake()
{
    std::cout << "Calling async_handshake" << std::endl;

    ssl_started_ = true;

    ssl_socket_.async_handshake(
            boost::asio::ssl::stream_base::server,
            boost::bind(
                &ExpectServerSsl::HandleHandshake,
                shared_from_this(),
                boost::asio::placeholders::error
            )
        );
}
