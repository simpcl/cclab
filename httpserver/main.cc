#include <iostream>

#include "tcp_server.h"
#include "http_server.h"

class MyTcpServer : public TcpServer {
 public:
  MyTcpServer() : TcpServer(8001) {}

 protected:
  virtual void OnMessage(TcpConnectionPtr conn, const std::string& msg, size_t bytes) {
    TcpServer::OnMessage(conn, msg, bytes);
    std::cout << "my tcp server" << std::endl;
    conn->AsyncRead();
  }
};

class MyHttpContext : public HttpContext {
 public:
  MyHttpContext(TcpConnectionPtr conn) : HttpContext(conn) {}

 protected:
  virtual void OnRequest() {
    std::cout << "OnRequest, request url: " << request_url_ << std::endl;
  }
  virtual void OnBody(const char *at, size_t len) {
    std::cout << "OnBody" << std::endl;
  }
  virtual void OnComplete() {
    std::cout << "OnComplete" << std::endl;
    auto conn = GetTcpConnectionPtr();
    conn->AsyncWrite(std::string("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n"));
  }
};

class MyHttpServer : public HttpServer {
 public:
  MyHttpServer() : HttpServer(8001) {}

 protected:
  HttpContextPtr CreateHttpContext(TcpConnectionPtr conn) {
    return boost::shared_ptr<MyHttpContext>(new MyHttpContext(conn));
  }
};


int main(int args, char** argv)
{
  std::cout << "hello http_test" << std::endl;

  //MyTcpServer tcp_server;
  //tcp_server.Start();
  //tcp_server.Wait();
  MyHttpServer *hs = new MyHttpServer();
  hs->Start();
  hs->Wait();
  return 0;
}
