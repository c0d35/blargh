/***
 * just simply a simple test of testing the test thing for testing purpose
 *
 * coder@computer.org
 * coder@0xc0d3.org
 *
 */
#ifndef RESTFOO_H
#define RESTFOO_H
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "blargh.h"

using namespace blargh;
static std::string data_root_ = "./data_dump/";

struct restfoo
{
    struct GET
    {
        ext2type e2t;
        void operator()(const request& req, response& resp)
        {

            boost::regex val_0 {"^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"};
            boost::smatch matches;
            if(!boost::regex_match(req.uri, matches, val_0))
            {
                resp = response::default_response(bad_request);
                return;
            }
            std::string path(matches[5]);

            if (path.empty() || path[0] != '/' || path.find("..") != std::string::npos)
            {
                resp = response::default_response(bad_request);
                return;
            }

            if (path[path.size() - 1] == '/')
            {
                path += "main.html";

            }

            std::size_t last_slash_pos = path.find_last_of("/");
            std::size_t last_dot_pos = path.find_last_of(".");
            std::string extension;
            if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
            {
                extension = path.substr(last_dot_pos + 1);
            }
            std::string full_path = doc_root_ + path;
            std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
            if (!is)
            {
                resp = response::default_response(status_type::not_found);
                return;
            }

            resp.status = status_type::ok;
            char buf[512];
            while (is.read(buf, sizeof(buf)).gcount() > 0)
                resp.content.append(buf, is.gcount());
            resp.headers.resize(2);
            resp.headers[0].name = "Content-Length";
            resp.headers[0].value = boost::lexical_cast<std::string>(resp.content.size());
            resp.headers[1].name = "Content-Type";
            resp.headers[1].value = e2t[extension];

        }
    };

    struct HEAD
    {
        void operator()(const request& req, response& resp)
        {
        }
    };

    struct POST
    {
        void operator()(const request& req, response& resp)
        {
            std::string filename(data_root_);
            boost::regex val_0 {"^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"};
            boost::smatch matches;

            if(!boost::regex_match(req.uri, matches, val_0))
            {
                resp = response::default_response(status_type::bad_request);
                return;
            }
            std::string path(matches[5]);
            
            if (path.empty() || path[0] != '/' || path.find("..") != std::string::npos)
            {
                resp = response::default_response(status_type::bad_request);
                return;
            }
            boost::regex val_1 {"\\[([-+]?[0-9]*\\.?[0-9]*),([-+]?[0-9]*\\.?[0-9]*),([-+]?[0-9]*\\.?[0-9]*),([-+]?[0-9]*\\.?[0-9]*)\\]"};
            if(!boost::regex_match(req.content, matches, val_1))return;
            resp = response::default_response(status_type::ok);

        }
    };

    struct PUT
    {
        void operator()(const request& req, response& resp)
        {
        }
    };

    struct DELETE
    {
        void operator()(const request& req, response& resp)
        {
        }
    };

    struct TRACE
    {
        void operator()(const request& req, response& resp)
        {
        }
    };

    struct CONNECT
    {
        void operator()(const request& req, response& resp)
        {}
    };

};
#endif
