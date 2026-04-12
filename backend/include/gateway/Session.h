#ifndef SESSION_H
#define SESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <jwt-cpp/jwt.h>
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

  bool validateJWT(const std::string& token, std::string& errorMsg);

  std::string base64Encode(const uint8_t* input, size_t length);
  std::string hashPassword(const std::string& password);
  bool verifyPassword(const std::string& password, const std::string& encoded_hash);

  boost::json::value handleLogin(const std::string& body);
  boost::json::value handleStatus();
  boost::json::value handleTransactions();

  std::string getEnvVar(const std::string& var, const std::string& def = "");
};

#endif
