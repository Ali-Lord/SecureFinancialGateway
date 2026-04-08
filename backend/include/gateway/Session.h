#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <memory>

class Session : public std::enable_shared_from_this<Session> {
  boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
  boost::beast::flat_buffer buffer_;
  boost::beast::http::request<boost::beast::http::string_body> req_;
  boost::asio::thread_pool& dbPool_;

public:
  explicit Session(
    boost::asio::ip::tcp::socket&& socket,
    boost::asio::ssl::context& ctx,
    boost::asio::thread_pool& dbPool
  );

  void run();

private:
  void onHandshake(boost::beast::error_code ec);
  void doRead();
  void onRead(boost::beast::error_code ec, std::size_t bytesTransferred);
  void handleRequest();
  void sendResponse(boost::beast::http::status status, std::string body);
  std::string getEnvVar(const std::string& var, const std::string& def);
};

#endif
