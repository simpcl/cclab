
#include "http_server.h"

int HttpContext::message_begin_cb(http_parser* parser) {
  HttpContext* ctx = reinterpret_cast<HttpContext*>(parser->data);
  ctx->http_method_ = static_cast<http_method>(parser->method);
  return 0;
}

int HttpContext::header_field_cb(http_parser* parser, const char *at, size_t length) {
  HttpContext* ctx = reinterpret_cast<HttpContext*>(parser->data);
  ctx->tmp_header_key_.append(at, length);
  return 0;
}

int HttpContext::header_value_cb(http_parser* parser, const char *at, size_t length) {
  HttpContext* ctx = reinterpret_cast<HttpContext*>(parser->data);
  std::string& k = ctx->tmp_header_key_;
  std::transform(k.begin(), k.end(), k.begin(), ::tolower);
  std::string v(at, length);
  if (k == "host") {
    const std::string& path = ctx->path_;
    size_t pos = path.find(v);
    size_t path_pos = 7 + v.size();
    if (pos != std::string::npos && path.size() > path_pos) {
      ctx->path_.assign(path.substr(path_pos));
    }
  }
  ctx->request_headers_.insert(std::make_pair(k, v));
  ctx->tmp_header_key_.clear();
  return 0;
}

int HttpContext::request_url_cb(http_parser* parser, const char *at, size_t length) {
  HttpContext* ctx = reinterpret_cast<HttpContext*>(parser->data);
  ctx->request_url_.assign(at, length);

  size_t i = 0;
  for (; i < length; i++) {
    char ch = *(at + i);
    if (ch == '?') {
      break;
    }
  }
  ctx->path_.assign(at, i);
  
  if (i < length) {
    std::string param_str(at + i + 1, length - i - 1);
    std::vector<std::string> elems;
    std::stringstream ss(param_str);
    std::string item;
    while (std::getline(ss, item, '&')) {
      if (!item.empty())
        elems.push_back(item);
    }
    for (const auto& e : elems) {
      size_t pos = e.find('=');
      std::string k, v;
      if (pos == std::string::npos) {
        k = e;                
      } else {
        k = e.substr(0, pos);
        v = e.substr(pos + 1);
      }
      ctx->query_params_.insert(std::make_pair(k, v));
    }
  } 
    
  return 0;
}

int HttpContext::headers_complete_cb(http_parser* parser) {
  HttpContext* ctx = reinterpret_cast<HttpContext*>(parser->data);
  ctx->content_length_ = parser->content_length;
  ctx->OnRequest();
  return 0;
}

int HttpContext::body_cb(http_parser* parser, const char *at, size_t length) {
  HttpContext* ctx = reinterpret_cast<HttpContext*>(parser->data);
  ctx->OnBody(at, length);
  return 0;
}

int HttpContext::message_complete_cb(http_parser* parser) {
  HttpContext* ctx = reinterpret_cast<HttpContext*>(parser->data);
  ctx->OnComplete();
  http_parser_init(parser, HTTP_REQUEST);
  return 0;
}

HttpContext::HttpContext(TcpConnectionPtr conn) : weak_conn_ptr_(conn) {
  http_parser_settings_init(&http_parser_settings_);
  http_parser_settings_.on_message_begin = message_begin_cb;
  http_parser_settings_.on_header_field = header_field_cb;
  http_parser_settings_.on_header_value = header_value_cb;
  http_parser_settings_.on_url = request_url_cb;
  http_parser_settings_.on_status = response_status_cb;
  http_parser_settings_.on_body = body_cb;
  http_parser_settings_.on_headers_complete = headers_complete_cb;
  http_parser_settings_.on_message_complete = message_complete_cb;
  //http_parser_settings_.on_chunk_header = chunk_header_cb;
  //http_parser_settings_.on_chunk_complete = chunk_complete_cb;

  http_parser_init(&http_parser_, HTTP_REQUEST);
  http_parser_.data = this;
}

void HttpContext::Parse(const std::string& msg, size_t bytes) {
  auto mem = input_buffer_.PreAllocate();
  void* data = mem.first;
  size_t len = mem.second;
  if (len < bytes) {
    return;
  }
  memcpy(data, (void *)msg.data(), bytes);
  //const char* data = input_buffer_.tail();
  input_buffer_.PostAllocate(bytes);
  int nparsed = http_parser_execute(&http_parser_, &http_parser_settings_, (const char *)data, bytes);
  input_buffer_.TrimStart(nparsed);
}

HttpContextPtr HttpServer::CreateHttpContext(TcpConnectionPtr conn) {
  return boost::make_shared<HttpContext>(conn);
}

void HttpServer::OnNewConnection(TcpConnectionPtr conn) {
  boost::shared_ptr<HttpContext> ctx = CreateHttpContext(conn);
  conn->SetContext(ctx);
  conn->AsyncRead();
}

void HttpServer::OnMessage(TcpConnectionPtr conn, const std::string& msg, size_t bytes) {
  boost::shared_ptr<HttpContext> ctx = boost::any_cast< boost::shared_ptr<HttpContext> >(conn->GetContext());
  if (ctx) {
    ctx->Parse(msg, bytes);
  }
}

