/*
 * @copyright Copyright 2014 Mediafire
 */

#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/http/post_data_pipe_interface.hpp"
#include "mediafire_sdk/http/error.hpp"

#include "mediafire_sdk/http/unit_tests/expect_server.hpp"
#include "mediafire_sdk/http/unit_tests/expect_server_ssl.hpp"

#include "mediafire_sdk/utils/base64.hpp"
#include "mediafire_sdk/utils/string.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#  include "boost/asio/impl/src.hpp"  // Define once in program
#  include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

namespace asio = boost::asio;


namespace {
    const uint16_t kPort1 = 49995;
    const uint16_t kPort2 = 49996;

    const std::string kHost = "127.0.0.1";

    std::shared_ptr<asio::io_service::work> MakeWork(
            asio::io_service * ios
            )
    {
        return std::make_shared<asio::io_service::work>(
                *ios
            );
    }

    // Variant types
    void SendRandomChunk(ExpectServerBase * server, std::size_t chunk_size)
    {
        // No hex chars here to avoid accidental success if chunking is broken.
        static std::string chars(
                "ghijklmnopqrstuvwxyz"
                "GHIJKLMNOPQRSTUVWXYZ"
                "!@#$%^&*()"
                "`~-_=+[{]{\\|;:'\",<.>/? ");

        static boost::random::random_device rng;
        static boost::random::uniform_int_distribution<> index_dist(
                0, chars.size() - 1);


        std::string data;
        {
            std::stringstream ss;
            ss << std::hex << chunk_size;
            data += ss.str();
        }
        data += "\r\n";
        data.reserve(data.size() + chunk_size + 2);

        for (std::size_t i = 0; i < chunk_size; ++i)
        {
            data.push_back( chars[index_dist(rng)] );
        }

        data += "\r\n";

        server->Push(expect_server_test::SendMessage( std::move(data) ));
    }

    void SendRandomContent(ExpectServerBase * server, std::size_t content_size)
    {
        static std::string chars(
                "abcdefghijklmnopqrstuvwxyz"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "1234567890"
                "!@#$%^&*()"
                "`~-_=+[{]{\\|;:'\",<.>/? ");

        static boost::random::random_device rng;
        static boost::random::uniform_int_distribution<> index_dist(
                0, chars.size() - 1);

        std::string data;
        data.reserve(content_size);

        for (std::size_t i = 0; i < content_size; ++i)
        {
            data.push_back( chars[index_dist(rng)] );
        }

        server->Push(expect_server_test::SendMessage( std::move(data) ));
    }

    enum Enc
    {
        Enc_None,
        Enc_Ssl,
    };

    std::string MakeUrl(
            Enc ssl,
            uint16_t port,
            std::string query
        )
    {
        std::string redirect_url;

        if ( ssl == Enc_Ssl )
            redirect_url += "https://";
        else
            redirect_url += "http://";
        redirect_url += kHost;
        redirect_url += ":";
        redirect_url += mf::utils::to_string(port);
        if ( ! query.empty() )
        {
            redirect_url += "/";
            redirect_url += query;
        }

        return redirect_url;
    }

    void SetProxies(
        mf::http::HttpConfig::Pointer http_config,
        uint16_t port,
        std::string username = "",
        std::string password = ""
    )

    {
        mf::http::Proxy proxy;

        proxy.host = kHost;
        proxy.port = port;
        proxy.username = username;
        proxy.password = password;

        http_config->SetHttpProxy(proxy);
        http_config->SetHttpsProxy(proxy);
    }

    class ResponseReader : public mf::http::RequestResponseInterface
    {
    public:
        ResponseReader() :
            success_(boost::logic::indeterminate)
        {}

        virtual void RedirectHeaderReceived(
                const mf::http::Headers & /* headers */,
                const mf::http::Url & /* new_url */
            ) override
        {
        }

        virtual void ResponseHeaderReceived(
                const mf::http::Headers & /* headers */
            ) override
        {
        }

        virtual void ResponseContentReceived(
                std::size_t /* start_pos */,
                std::shared_ptr<mf::http::BufferInterface> buffer
            ) override
        {
            std::cout << "Read " << buffer->Size() << " bytes." << std::endl;
        }

        virtual void RequestResponseErrorEvent(
                std::error_code error_code,
                std::string error_text
            ) override
        {
            std::cerr << "Error: " << error_code.message() << '\n'
                << "Error message: " << error_text << std::endl;
            success_ = false;
        }

        virtual void RequestResponseCompleteEvent() override
        {
            if ( boost::logic::indeterminate(success_) )
                success_ = true;
        }

        bool Success()
        {
            return (success_ == true);
        }

    private:
        boost::tribool success_;
    };
}  // namespace

bool TestTimeout()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
                );

    // Set the action timeout. Should be a little more than the IO timeout we
    // set, so that we hit the IO timeout but not the action timeout, assuming
    // the IO timeout works.
    server->SetActionTimeoutMs(2000);

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    // Expect a bunch of data.
    server->Push(expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Content-Length: 200000\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    // But only send a little.
    uint64_t total_content = 0;
#define SendSome(x) SendRandomContent( server.get(), x ); total_content += x;
    SendSome( 20 );
    SendSome( 5 );
    SendSome( 100 );
    SendSome( 13 );
    SendSome( 0 );  // Terminates connection
#undef SendSome

    // and wait till we get the timeout...
    server->Push( ExpectError{mf::http::http_error::IoTimeout} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    auto request = mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "") );

    // Set timeout to one second.
    request->SetTimeout( 1 );

    // Start the request.
    request->Start();

    try
    {
        io_service.run();
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }

    return server->Success();
}

bool TestChunked()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
                );

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push(expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server.get(), x ); total_content += x;
    ChunkSend( 20 );
    ChunkSend( 5 );
    ChunkSend( 100 );
    ChunkSend( 13 );
    ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestBigChunked()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    // This is a big one.
    server->SetActionTimeoutMs(30*1000);

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push(expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
    for ( unsigned int i = 0; i < 10; ++i )
    {
        SendRandomChunk( server.get(), 100000 );
        total_content += 100000;
    }
    SendRandomChunk( server.get(), 0 );  // Terminates connection

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestContentLength()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push(expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Content-Length: 2000\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    SendRandomContent( server.get(), 2000 );

    server->Push( ExpectDisconnect{2000} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestContentLengthEmpty()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push(expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Content-Length: 0\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    server->Push( ExpectDisconnect{static_cast<std::size_t>(0)} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestBigContentLength()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push(expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Content-Length: 200000\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    for ( int i = 0; i < 100; ++i )
    {
        SendRandomContent( server.get(), 2000 );
    }

    server->Push( ExpectDisconnect{200000} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestBigContentLength2()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push(expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Content-Length: 200000\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    for ( int i = 0; i < 10; ++i )
    {
        SendRandomContent( server.get(), 20000 );
    }

    server->Push( ExpectDisconnect{200000} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestPost()
{
    asio::io_service io_service;

    mf::http::SharedBuffer::Pointer shared_buffer(
        mf::http::SharedBuffer::Create(
            std::string( "This is a simple POST data." )));

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    server->Push( ExpectRegex{ boost::regex(
            "POST.*\r\n"
            "\r\n"
        )});

    server->Push( ExpectContentLength{static_cast<size_t>(shared_buffer->Size())} );

    server->Push(expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server.get(), x ); total_content += x;
    ChunkSend( 5 );
    ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Add data to send as POST.
    request->SetPostData(shared_buffer);

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

class PostDataPipe :
    public mf::http::PostDataPipeInterface
{
public:
    PostDataPipe() {}
    virtual ~PostDataPipe() {}

    virtual uint64_t PostDataSize() const
    {
        uint64_t ret = 0;
        for ( auto & it : vec_ )
        {
            ret += it->Size();
        }
        return ret;
    }

    virtual mf::http::SharedBuffer::Pointer RetreivePostDataChunk()
    {
        if ( vec_.empty() )
            return mf::http::SharedBuffer::Pointer();
        else
        {
            mf::http::SharedBuffer::Pointer shared_buffer = vec_.front();
            vec_.pop_front();
            return shared_buffer;
        }
    }

    void PushData( mf::http::SharedBuffer::Pointer shared_buffer )
    {
        vec_.push_back(shared_buffer);
    }

private:
    std::list<mf::http::SharedBuffer::Pointer> vec_;
};

bool TestPostPipe()
{
    const int repetitions = 5;

    asio::io_service io_service;

    mf::http::SharedBuffer::Pointer shared_buffer(
        mf::http::SharedBuffer::Create(
            std::string("This is a simple POST data.") ) );

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    server->Push( ExpectRegex{ boost::regex(
            "POST.*\r\n"
            "\r\n"
        )});

    server->Push( ExpectContentLength{static_cast<size_t>(shared_buffer->Size() * repetitions)} );

    server->Push(expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server.get(), x ); total_content += x;
    ChunkSend( 5 );
    ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    // Make this slow
    http_config->SetBandwidthUsagePercent(2);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Add data to send as POST.
    std::shared_ptr<PostDataPipe> pdp( std::make_shared<PostDataPipe>() );

    // Load up some data to pipe.
    for ( int i = 0; i < repetitions; ++i )
        pdp->PushData(shared_buffer);

    request->SetPostDataPipe(pdp);

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestHttpRedirectPermission()
{
    asio::io_service io_service;

    std::shared_ptr<asio::io_service::work> work = MakeWork(&io_service);

    // Server 1

    // First server with redirect.
    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                work,
                kPort1
            );

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    {
        std::string header;
        header += "HTTP/1.1 301 Moved Permanently\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n";
        header += "Location: " + MakeUrl(Enc_None, kPort2, "") + "\r\n";
        header +=
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n";

        server->Push(expect_server_test::SendMessage(
                header
            ));
    }

    // Send some junk and disconnect
    {
        uint64_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server.get(), x ); total_content += x;
        ChunkSend( 5 );
        ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    }

    server->Push( ExpectError{mf::http::http_error::RedirectPermissionDenied} );

    // Connection

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    // Deny redirects
    http_config->SetRedirectPolicy(mf::http::RedirectPolicy::Deny);
    // We don't allow redirects, we want to fail.

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    work.reset();

    io_service.run();

    // Server 2 should time out.
    return server->Success();
}

bool TestHttpRedirect301()
{
    asio::io_service io_service;

    std::shared_ptr<asio::io_service::work> work = MakeWork(&io_service);

    // Server 1

    // First server with redirect.
    std::shared_ptr<ExpectServer> server1 =
        ExpectServer::Create(
                &io_service,
                work,
                kPort1
            );

    server1->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    {
        std::string header;
        header += "HTTP/1.1 301 Moved Permanently\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n";
        header += "Location: " + MakeUrl(Enc_None, kPort2, "") + "\r\n";
        header +=
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n";

        server1->Push(expect_server_test::SendMessage(
                header
            ));
        // server1->Push( ExpectRedirect{} );
    }

    // Send some junk and disconnect
    {
        uint64_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server1.get(), x ); total_content += x;
        ChunkSend( 5 );
        ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

        // server1->Push( ExpectDisconnect{total_content} );
    }

    // Server 2

    // Second server with non-redirect
    std::shared_ptr<ExpectServer> server2 =
        ExpectServer::Create(
                &io_service,
                work,
                kPort2
            );

    server2->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server2->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server1->Push( ExpectHeadersRead{} );

    {
        std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server2.get(), x ); total_content += x;
        ChunkSend( 20 );
        ChunkSend( 5 );
        ChunkSend( 100 );
        ChunkSend( 13 );
        ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

        server1->Push( ExpectDisconnect{total_content} );
    }

    // Connection

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    // Allow redirects
    http_config->SetRedirectPolicy(mf::http::RedirectPolicy::DenyDowngrade);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server1),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    work.reset();

    io_service.run();

    return server1->Success() && server2->Success();
}

bool TestHttpRedirect302()
{
    asio::io_service io_service;

    std::shared_ptr<asio::io_service::work> work = MakeWork(&io_service);

    // Server 1

    // First server with redirect.
    std::shared_ptr<ExpectServer> server1 =
        ExpectServer::Create(
                &io_service,
                work,
                kPort1
            );

    server1->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    {
        std::string header;
        header += "HTTP/1.1 302 Found\r\n";
        header += "Location: " + MakeUrl(Enc_None, kPort2, "") + "\r\n";
        header += "\r\n";

        server1->Push( expect_server_test::SendMessage(
                header
            ));
        // server1->Push( ExpectRedirect{} );
    }

    // Send some junk and disconnect
    {
        uint64_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server1.get(), x ); total_content += x;
        ChunkSend( 5 );
        ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

        // server1->Push( ExpectDisconnect{total_content} );
    }

    // Server 2

    // Second server with non-redirect
    std::shared_ptr<ExpectServer> server2 =
        ExpectServer::Create(
                &io_service,
                work,
                kPort2
            );

    server2->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server2->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server1->Push( ExpectHeadersRead{} );

    {
        std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server2.get(), x ); total_content += x;
        ChunkSend( 20 );
        ChunkSend( 5 );
        ChunkSend( 100 );
        ChunkSend( 13 );
        ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

        server1->Push( ExpectDisconnect{total_content} );
    }

    // Connection

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    // Allow redirects
    http_config->SetRedirectPolicy(mf::http::RedirectPolicy::DenyDowngrade);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server1),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    work.reset();

    io_service.run();

    return server1->Success() && server2->Success();
}

bool TestFailSelfSignedSsl()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServerSsl> server =
        ExpectServerSsl::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    server->Push( ExpectHandshake{} );

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server.get(), x ); total_content += x;
    ChunkSend( 20 );
    ChunkSend( 5 );
    ChunkSend( 100 );
    ChunkSend( 13 );
    ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_Ssl, kPort1, "")
        ));

    // Our certificate is self signed, but we don't call this because we are
    // testing this functionality.
    // request->AllowSelfSignedCertificate();

    // Start the request.
    request->Start();

    io_service.run();

    std::cout << "Error: " << server->Error().message() << std::endl;

    return server->Error() == mf::http::http_error::SslHandshakeFailure;
}

bool TestChunkedSsl()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServerSsl> server =
        ExpectServerSsl::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    server->Push( ExpectHandshake{} );

    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server.get(), x ); total_content += x;
    ChunkSend( 20 );
    ChunkSend( 5 );
    ChunkSend( 100 );
    ChunkSend( 13 );
    ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    // Our certificate is self signed.
    http_config->AllowSelfSignedCertificate();

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_Ssl, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestMediafireSsl()
{
    asio::io_service io_service;

    std::shared_ptr<ResponseReader> rr(
            std::make_shared<ResponseReader>()
        );

    std::string host = "https://www.mediafire.com";

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(rr),
                host
        ));

    std::cout << "Contacting host: " << host << std::endl;

    // Start the request.
    request->Start();

    io_service.run();

    if ( ! rr->Success() )
    {
        std::cout << "Unable to connect to MediaFire live via SSL." << std::endl;
        return false;
    }
    return true;
}

bool TestHttpProxy()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    // Simple proxy sends uri instead of query.
    server->Push( ExpectRegex{ boost::regex(
            "GET http://.*\r\n"
            "\r\n"
        )});

    server->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server.get(), x ); total_content += x;
    ChunkSend( 20 );
    ChunkSend( 5 );
    ChunkSend( 100 );
    ChunkSend( 13 );
    ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    SetProxies(http_config, kPort1);
    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestHttpProxyLogin()
{
    const std::string username = "test_user";
    const std::string password = "123456";

    const std::string auth_line = [&]()
    {
        std::stringstream ss;
        std::string to_encode = username;
        to_encode += ':';
        to_encode += password;
        std::string encoded = mf::utils::Base64Encode(
                to_encode.c_str(), to_encode.size() );

        ss << "Proxy-Authorization: Basic " << encoded;
        return ss.str();
    }();

    asio::io_service io_service;

    std::shared_ptr<ExpectServer> server =
        ExpectServer::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    // Simple proxy sends uri instead of query, along with authorization.
    std::string connect_regex;
    connect_regex += "CONNECT ";  // Authentication over proxy requires CONNECT
    connect_regex += ".*";        // Hostname and anything till
    connect_regex += auth_line;
    connect_regex += ".*";
    connect_regex += "\r\n\r\n";  // The header end.
    server->Push( ExpectRegex{ boost::regex( connect_regex ) } );

    // Send the proxy OK response.
    server->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "\r\n"
        ));

    // Proceed as normally.
    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server.get(), x ); total_content += x;
    ChunkSend( 20 );
    ChunkSend( 5 );
    ChunkSend( 100 );
    ChunkSend( 13 );
    ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();

    http_config->SetWorkIoService(&io_service);

    SetProxies(http_config, kPort1, username, password);
    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_None, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestHttpsProxy()
{
    asio::io_service io_service;

    std::shared_ptr<ExpectServerSsl> server =
        ExpectServerSsl::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    // Simple proxy sends uri instead of query.
    std::string connect_regex;
    connect_regex += "CONNECT ";  // SSL over proxy requires passthrough
    connect_regex += kHost;       // Then the host name (and https port)
    connect_regex += ".*";        // Then anything till
    connect_regex += "\r\n\r\n";  // The header end.
    server->Push( ExpectRegex{ boost::regex( connect_regex ) } );

    // Send the proxy OK response.
    server->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "\r\n"
        ));

    // Now perform the SSL handshake.
    server->Push( ExpectHandshake{} );

    // Proceed as normally.
    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk ( server.get(), x ); total_content += x;
    ChunkSend( 20 );
    ChunkSend( 5 );
    ChunkSend( 100 );
    ChunkSend( 13 );
    ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    // Our certificate is self signed.
    http_config->AllowSelfSignedCertificate();

    SetProxies(http_config, kPort1);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_Ssl, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

bool TestHttpsProxyLogin()
{
    const std::string username = "test_user";
    const std::string password = "123456";

    const std::string auth_line = [&]()
    {
        std::stringstream ss;
        std::string to_encode = username;
        to_encode += ':';
        to_encode += password;
        std::string encoded = mf::utils::Base64Encode(
                to_encode.c_str(), to_encode.size() );

        ss << "Proxy-Authorization: Basic " << encoded;
        return ss.str();
    }();

    asio::io_service io_service;

    std::shared_ptr<ExpectServerSsl> server =
        ExpectServerSsl::Create(
                &io_service,
                MakeWork(&io_service),
                kPort1
            );

    // Simple proxy sends uri instead of query.
    std::string connect_regex;
    connect_regex += "CONNECT ";  // SSL over proxy requires passthrough
    connect_regex += kHost;       // Then the host name (and https port)
    connect_regex += ".*";        // Then anything till
    connect_regex += auth_line;
    connect_regex += ".*";
    connect_regex += "\r\n\r\n";  // The header end.
    server->Push( ExpectRegex{ boost::regex( connect_regex ) } );

    // Send the proxy OK response.
    server->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "\r\n"
        ));

    // Now perform the SSL handshake.
    server->Push( ExpectHandshake{} );

    // Proceed as normally.
    server->Push( ExpectRegex{ boost::regex(
            "GET.*\r\n"
            "\r\n"
        )});

    server->Push( expect_server_test::SendMessage(
            "HTTP/1.1 200 OK\r\n"
            "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n"
            "Server: Apache\r\n"
            "Cache-control: no-cache, must-revalidate\r\n"
            "Pragma: no-cache\r\n"
            "Expires: 0\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
        ));
    server->Push( ExpectHeadersRead{} );

    std::size_t total_content = 0;
#define ChunkSend(x) SendRandomChunk( server.get(), x ); total_content += x;
    ChunkSend( 20 );
    ChunkSend( 5 );
    ChunkSend( 100 );
    ChunkSend( 13 );
    ChunkSend( 0 );  // Terminates connection
#undef ChunkSend

    server->Push( ExpectDisconnect{total_content} );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    // Our certificate is self signed.
    http_config->AllowSelfSignedCertificate();

    SetProxies(http_config, kPort1, username, password);

    mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                    mf::http::RequestResponseInterface>(server),
                MakeUrl(Enc_Ssl, kPort1, "")
        ));

    // Start the request.
    request->Start();

    io_service.run();

    return server->Success();
}

int main(int /* argc */, char* /* argv */[])
{
    bool success = true;
    std::vector< std::pair< std::string, bool> > total_result;

#define TEST(x) test(x, #x)
    // Why? Cause cpplint hated my macro.
    auto test = [&](bool (*x)(), std::string name)
    {
        std::cout << "- " << name << ' ' << std::string(69 - name.size(), '-')
            << std::endl;
        try
        {
            if ( ! x() )
            {
                std::cerr << "Failed " << name << std::endl;
                success = false;
                total_result.push_back( std::make_pair(name, false) );
            }
            else
            {
                total_result.push_back( std::make_pair(name, true) );
            }
        }
        catch(std::exception & ex)
        {
            std::cout << "Uncaught exception: " << ex.what() << std::endl;
            success = false;
            total_result.push_back( std::make_pair(name, false) );
        }
        std::cout << std::endl;
    };

    TEST(TestTimeout);
    TEST(TestChunked);
    TEST(TestBigChunked);

    TEST(TestContentLength);
    TEST(TestContentLengthEmpty);
    TEST(TestBigContentLength);
    TEST(TestBigContentLength2);

    TEST(TestPost);
    TEST(TestPostPipe);

    TEST(TestHttpRedirectPermission);
    TEST(TestHttpRedirect301);
    TEST(TestHttpRedirect302);

    TEST(TestFailSelfSignedSsl);
    TEST(TestChunkedSsl);

    TEST(TestHttpProxy);
    TEST(TestHttpProxyLogin);

    TEST(TestHttpsProxy);
    TEST(TestHttpsProxyLogin);

    TEST(TestMediafireSsl);  // This will fail if mediafire is down.

    // Make nice summary:
    std::string::size_type max_width = 10;
    for ( const auto & pair : total_result )
    {
        if ( pair.first.size() > max_width )
            max_width = pair.first.size();
    }
    max_width += 2;
    std::cout << "- Summary " << std::string(max_width, '-') << std::endl;
    int count = 0;
    for ( const auto & pair : total_result )
    {
        ++count;
        std::cout << std::left << std::setw(2) << count << ' ';

        std::cout << std::left << std::setw(max_width) << (pair.first+':');

        if ( pair.second )
            std::cout << "SUCCESS" << std::endl;
        else
            std::cout << "   FAIL" << std::endl;
    }

    if ( ! success )
        return 1;
    return 0;
}
