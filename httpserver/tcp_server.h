#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <iostream>
#include <boost/any.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include "iobuf.h"

using namespace boost;

typedef boost::system::error_code boost_error_code;

class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef boost::function<void (TcpConnectionPtr, const boost_error_code &)> ConnectionCallback;
typedef boost::function<void (TcpConnectionPtr, const std::string& msg, size_t bytes)> MessageCallback;
typedef boost::function<void (TcpConnectionPtr, size_t bytes)> WriteCompleteCallback;

class TcpConnection : public boost::enable_shared_from_this<TcpConnection> {
 public:
  static TcpConnectionPtr CreateConnection(asio::io_service& service);

  void AsyncRead();
  void AsyncWrite(const std::string & msg);

  void Close() { sock_.close(); }

  asio::ip::tcp::socket & Sock() { return sock_;}

  void SetContext(const boost::any& context)
  { context_ = context; }

  boost::any GetContext()
  { return context_; }

  void SetConnectionCallback(const ConnectionCallback& cb) { connection_cb_ = cb; }
  void SetMessageCallback(const MessageCallback& cb) { message_cb_ = cb; }
  void SetWriteCompleteCallback(const WriteCompleteCallback& cb) { write_complete_cb_ = cb; }

 protected:
  //virtual void OnAccepted() {}
  void OnRead(const boost_error_code & ec, size_t bytes);
  void OnWrite(const boost_error_code & ec, size_t bytes);

 private:
  TcpConnection(asio::io_service& service);
  TcpConnection(const TcpConnection&) = delete;

 private:
  asio::ip::tcp::socket sock_;
  enum { max_msg = 4096 };
  char read_buffer_[max_msg];
  char write_buffer_[max_msg];
  boost::any context_;

 private:
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  WriteCompleteCallback write_complete_cb_;
};

class TcpServer {
 public:
  TcpServer(int port) : acceptor_(service_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
  {}
  TcpServer(const TcpServer &) = delete;
  virtual ~TcpServer() {}

  void Start();
  void Wait();

 protected:
  virtual void OnNewConnection(TcpConnectionPtr conn) {
    conn->AsyncRead();
  }
  virtual void OnConnectionError(TcpConnectionPtr conn, const boost_error_code& ec) {
    //std::cout << "connection error, error code: " << ec << std::endl;
    conn->Close();
  }
  virtual void OnMessage(TcpConnectionPtr conn, const std::string& msg, size_t bytes) {
    std::cout <<"message callback, message: [" << msg << "], bytes: [" << bytes << "]" << std::endl;
  }
  virtual void OnWriteComplete(TcpConnectionPtr conn, size_t bytes) {
    //std::cout << "write complete callback" << std::endl;
  }

 private:
  void HandleAccept(TcpConnectionPtr conn, const boost_error_code & ec);

 private:
  asio::io_service service_;
  asio::ip::tcp::acceptor acceptor_;
  boost::thread_group threads_;
};

#endif // _TCP_SERVER_H_
