#pragma once
// #include <string.h>
#include <stdint.h>

class Request
{
public:
  Request(const httplib::Request& r):
      request_(r)
  {
  }

  std::string Body() const
  {
    return request_.body;
  }

  std::string Param(std::string_view name) const
  {
    auto it = request_.params.find(std::string(name));

    if (it == request_.params.end())
    {
      return {};
    }

    return it->second;
  }

private:
  const httplib::Request& request_;
};

class Response
{
public:
  static Response Json(std::string body)
  {
    return Content(std::move(body), "application/json");
  }

  static Response Error(int status, std::string body)
  {
    Response r    = Json(std::move(body));
    r.status      = status;
    return r;
  }

  static Response Content(std::string body, std::string contentType)
  {
    Response r;
    r.body        = std::move(body);
    r.contentType = std::move(contentType);
    r.status      = 200;
    return r;
  }

  static Response Content(std::string_view body, std::string_view contentType)
  {
    return Content(std::string(body), std::string(contentType));
  }

  void Write(httplib::Response& res) const
  {
    res.status = status;
    res.set_content(body, contentType);
  }

private:
  int status = 200;

  std::string body;
  std::string contentType;
};

using RequestHandler = std::function<Response(const Request&)>;

class iRESTServer
{
public:
  virtual ~iRESTServer() = default;

  virtual void Get(std::string_view route, RequestHandler handler) = 0;

  virtual void Post(std::string_view route, RequestHandler handler) = 0;

  virtual void Delete(std::string_view route, RequestHandler handler) = 0;

  virtual void Start(uint16_t port) = 0;
  virtual void Stop(void)           = 0;
};