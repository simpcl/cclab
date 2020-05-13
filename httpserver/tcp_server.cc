
#include <thread>

#include "tcp_server.h"


TcpConnection::TcpConnection(asio::io_service& service)
    : sock_(service) {
}

TcpConnectionPtr TcpConnection::CreateConnection(asio::io_service& service) {
  return boost::shared_ptr<TcpConnection>(new TcpConnection(service));
}

void TcpConnection::AsyncRead() {
  sock_.async_read_some(asio::buffer(read_buffer_), boost::bind(&TcpConnection::OnRead, shared_from_this(), _1, _2));
}

void TcpConnection::AsyncWrite(const std::string& msg) {
  std::copy(msg.begin(), msg.end(), write_buffer_);
  sock_.async_write_some(asio::buffer(write_buffer_, msg.size()),
      boost::bind(&TcpConnection::OnWrite, shared_from_this(), _1, _2));
}

void TcpConnection::OnRead(const boost_error_code & ec, size_t bytes) {
  if (ec) {
    connection_cb_(shared_from_this(), ec);
    return;
  }
  std::string msg(read_buffer_, bytes);
  message_cb_(shared_from_this(), msg, bytes);
}

void TcpConnection::OnWrite(const boost_error_code & ec, size_t bytes) {
  if (ec) {
    connection_cb_(shared_from_this(), ec);
    return;
  }
  write_complete_cb_(shared_from_this(), bytes);
}


void TcpServer::Start() {
  TcpConnectionPtr new_conn = TcpConnection::CreateConnection(service_);
  acceptor_.async_accept(new_conn->Sock(), boost::bind(&TcpServer::HandleAccept, this, new_conn, _1));

  for (int i=0; i<3; i++) {
    threads_.create_thread(boost::bind(&boost::asio::io_service::run, &service_));
  }
}

void TcpServer::Wait() {
  //service_.stop();
  threads_.join_all();
}

void TcpServer::HandleAccept(TcpConnectionPtr conn, const boost_error_code & ec) {
  std::cout << "handle accept, thread: " << std::this_thread::get_id() << std::endl;
  TcpConnectionPtr new_conn = TcpConnection::CreateConnection(service_);
  acceptor_.async_accept(new_conn->Sock(), boost::bind(&TcpServer::HandleAccept, this, new_conn, _1));
  conn->SetConnectionCallback(boost::bind(&TcpServer::OnConnectionError, this, _1, _2));
  conn->SetMessageCallback(boost::bind(&TcpServer::OnMessage, this, _1, _2, _3));
  conn->SetWriteCompleteCallback(boost::bind(&TcpServer::OnWriteComplete, this, _1, _2));
  OnNewConnection(conn);
}
