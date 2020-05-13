#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <iostream>
#include <string>
#include <unordered_map>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "tcp_server.h"
#include "http_parser.h"
#include "iobuf.h"

using namespace boost;

class HttpContext;
typedef boost::shared_ptr<HttpContext> HttpContextPtr;

class HttpContext : public boost::enable_shared_from_this<HttpContext> {
 public:
  HttpContext(TcpConnectionPtr conn);
  HttpContext(const HttpContext &) = delete;
  virtual ~HttpContext() {}

 public:
  void Parse(const std::string& msg, size_t bytes);
  HttpContextPtr GetPtr() { return shared_from_this(); }
  TcpConnectionPtr GetTcpConnectionPtr() { return weak_conn_ptr_.lock(); }

 protected:
  virtual void OnRequest() {
  }
  virtual void OnBody(const char *at, size_t len) {
  }
  virtual void OnComplete() {
    auto conn = GetTcpConnectionPtr();
    conn->AsyncWrite(std::string("HTTP/1.1 404 OK\r\nContent-Length: 0\r\n\r\n"));
  }

 public:
  http_method http_method_;
  std::string request_url_;
  std::string path_;
  size_t content_length_;
  //std::string content_type_;
  //bool require_100continue_;
  std::map<std::string, std::string> query_params_;
  std::map<std::string, std::string> request_headers_;

 private:
  http_parser_settings http_parser_settings_;
  http_parser http_parser_;
  std::string tmp_header_key_;
  IOBuf input_buffer_;
  boost::weak_ptr<TcpConnection> weak_conn_ptr_;

 private:
  static int message_begin_cb(http_parser* parser);
  static int header_field_cb(http_parser* parser, const char *at, size_t length);
  static int header_value_cb(http_parser* parser, const char *at, size_t length);
  static int request_url_cb(http_parser* parser, const char *at, size_t length);
  static int response_status_cb(http_parser* parser, const char *at, size_t length) { return 0; }
  static int body_cb(http_parser* parser, const char *at, size_t length);
  static int headers_complete_cb(http_parser* parser);
  static int message_complete_cb(http_parser* parser);
  //static int chunk_header_cb(http_parser* parser);
  //static int chunk_complete_cb(http_parser* parser);
};

class HttpServer : public TcpServer {
 public:
  HttpServer(int port) : TcpServer(port) {}
  HttpServer(const HttpServer &) = delete;
  virtual ~HttpServer() {}

 protected:
  virtual HttpContextPtr CreateHttpContext(TcpConnectionPtr conn);
  virtual void OnNewConnection(TcpConnectionPtr conn);
  virtual void OnMessage(TcpConnectionPtr conn, const std::string& msg, size_t bytes);
};

#endif // _HTTP_SERVER_H_
