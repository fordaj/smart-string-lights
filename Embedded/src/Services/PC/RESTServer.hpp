#pragma once
#include "httplib.h"
#include "iRESTServer.hpp"

class RESTServer : public iRESTServer
{
public:
  void Get(std::string_view route, RequestHandler handler) override
  {
    server_.Get(std::string(route),

                [handler](const httplib::Request& req, httplib::Response& res)
                {
                  Request r(req);

                  auto response = handler(r);

                  response.Write(res);
                });
  }

  void Post(std::string_view route, RequestHandler handler) override
  {
    server_.Post(std::string(route),

                 [handler](const httplib::Request& req, httplib::Response& res)
                 {
                   Request r(req);

                   auto response = handler(r);

                   response.Write(res);
                 });
  }

  void Delete(std::string_view route, RequestHandler handler) override
  {
    server_.Delete(std::string(route),

                   [handler](const httplib::Request& req, httplib::Response& res)
                   {
                     Request r(req);

                     auto response = handler(r);

                     response.Write(res);
                   });
  }

  void Start(uint16_t port) override
  {
    server_.listen("0.0.0.0", port);
  }

  void Stop(void) override
  {
    server_.stop();
  }

private:
  httplib::Server server_;
};