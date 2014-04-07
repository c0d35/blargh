/***
 * just simply a simple test of testing the test thing for testing purpose
 *
 * coder@computer.org
 * coder@0xc0d3.org
 *
 */
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <string>
#include <stack>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/logic/tribool.hpp>
#include <signal.h>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>

#include "blargh.h"

namespace blargh{

#include <boost/asio/yield.hpp>

    //first match HTTP... or extract methods?.
    //header processing is copypasta from the boost examplex
    boost::tribool req_parser::process_headers(request& req, char c)
    {
        reenter (this)
        {
            req.http_version_major = 0;
            req.http_version_minor = 0;
            req.headers.clear();
            req.content.clear();
            content_length_ = 0;
            while ((is_char(c) && !iscntrl(c) && !is_tspecial(c) && c != '\r')
                    || (c == ' ' || c == '\t'))
            {
                if (c == ' ' || c == '\t')
                {
                    if (req.headers.empty()) return false;
                    while (c == ' ' || c == '\t')
                        yield return boost::indeterminate;
                }
                else
                {
                    req.headers.push_back(header());

                    while (is_char(c) && !iscntrl(c) && !is_tspecial(c) && c != ':')
                    {
                        req.headers.back().name.push_back(c);
                        yield return boost::indeterminate;
                    }

                    if (c != ':') return false;
                    yield return boost::indeterminate;
                    if (c != ' ') return false;
                    yield return boost::indeterminate;
                }

                while (is_char(c) && !iscntrl(c) && c != '\r')
                {
                    req.headers.back().value.push_back(c);
                    yield return boost::indeterminate;
                }

                if (c != '\r') return false;
                yield return boost::indeterminate;
                if (c != '\n') return false;
                yield return boost::indeterminate;
            }

            if (c != '\r') return false;
            yield return boost::indeterminate;
            if (c != '\n') return false;

            for (std::size_t i = 0; i < req.headers.size(); ++i)
            {
                if(req.headers[i].name == "Content-Length")
                {
                    try
                    {
                        content_length_ = std::stoi(req.headers[i].value);
                    }
                    catch (...)//invalid cast ... out of range
                    {
                        return false;
                    }
                }
            }

            while (req.content.size() < content_length_)
            {
                yield return boost::indeterminate;
                req.content.push_back(c);
            }
        }

        return true;
    }

#include <boost/asio/unyield.hpp>

    bool req_parser::is_char(int c)
    {
        return c >= 0 && c <= 127;
    }


    bool req_parser::is_tspecial(int c)
    {
        switch (c)
        {
            case '(': case ')': case '<': case '>': case '@':
            case ',': case ';': case ':': case '\\': case '"':
            case '/': case '[': case ']': case '?': case '=':
            case '{': case '}': case ' ': case '\t':
                return true;
            default:
                return false;
        }
    }


    const std::string cat = "<pre>\n                |\      _,,,---,,_       \n                /,`.-'`'    -.  ;-;;,_   \n               |,4-  ) )-,_..;\ (  `'-'  \n              '---''(_/--'  `-'\_)       \n	    </pre>";        
    char kvsep[] = { ':', ' ' };
    char crlf[] = { '\r', '\n' };

    response response::default_response(status_type status)
    {

        std::string resp_str;
        if(status != status_type::ok)
        {
            resp_str = "<html><head><title>" + std::to_string(status) + "</title>" + "</head>" + "<body>" + cat + "\n <img src=\"./blargh.gif\"/>" + " </body>" + "</html>";
        }
        response resp;
        resp.status = status;
        resp.content = resp_str;
        resp.headers.resize(2);
        resp.headers[0].name = "Content-Length";
        resp.headers[0].value = boost::lexical_cast<std::string>(resp.content.size());
        resp.headers[1].name = "Content-Type";
        resp.headers[1].value = "text/html";
        return resp;
    }

    std::vector<boost::asio::const_buffer> response::to_buffers()
    {

        std::vector<boost::asio::const_buffer> v_b;
        v_b.push_back(boost::asio::buffer((*this)[status_type::ok]));
        std::for_each(headers.begin(), headers.end(), [&](header &hd)
                { 
                v_b.push_back(boost::asio::buffer(hd.name));
                v_b.push_back(boost::asio::buffer(kvsep));
                v_b.push_back(boost::asio::buffer(hd.value));
                v_b.push_back(boost::asio::buffer(crlf));

                });
        v_b.push_back(boost::asio::buffer(crlf));
        v_b.push_back(boost::asio::buffer(content));
        return v_b;
    }



    blarghd::blarghd(boost::asio::io_service& io_service, const std::string& ip_address, const std::string& port, boost::function<void(const request&, response&)> req_handler)
        : m_req_handler(req_handler)
    {
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(ip_address, port);
        m_acceptor.reset(new tcp::acceptor(io_service, *resolver.resolve(query)));
    }

#include <boost/asio/yield.hpp>
    //for details look into the boost coroutine documentation and consult their examples, we simply use the example code for rapid prototyping
    void blarghd::operator()(boost::system::error_code ec, std::size_t length)
    {
        if (!ec)
        {
            reenter (this)
            {
                do
                {
                    m_socket.reset(new tcp::socket(m_acceptor->get_io_service()));
                    yield m_acceptor->async_accept(*m_socket, *this);
                    fork blarghd(*this)();

                } while (is_parent());

                m_buffer.reset(new boost::array<char, 8192>);
                m_req.reset(new request);

                do
                {
                    yield m_socket->async_read_some(boost::asio::buffer(*m_buffer), *this);

                    boost::tie(m_valid_req, boost::tuples::ignore)
                        = m_req_parser.parse(*m_req,
                                m_buffer->data(), m_buffer->data() + length);

                } while (boost::indeterminate(m_valid_req));

                m_response.reset(new response);

                if (m_valid_req)
                {
                    m_req_handler(*m_req, *m_response);
                }
                else
                {
                    *m_response = response::default_response(status_type::bad_request);
                }

                yield boost::asio::async_write(*m_socket, m_response->to_buffers(), *this);

                m_socket->shutdown(tcp::socket::shutdown_both, ec);
            }
        }

    }

#include <boost/asio/unyield.hpp>


}

#include "restfoo.h"

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 4)
        {
            std::cerr << "Usage: blarghd <address> <port> <doc_root>" << std::endl;
            return 1;
        }
        boost::asio::io_service io_service;

        blargh::blarghd(io_service, argv[1], argv[2], blargh::REST_handler< restfoo >(argv[3]))();

        boost::asio::signal_set signal_set(io_service);
        signal_set.add(SIGINT);
        signal_set.add(SIGTERM);
        signal_set.add(SIGQUIT);
        signal_set.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service));

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "He's Dead Jim" << std::endl;
    }
    return 0;
}
