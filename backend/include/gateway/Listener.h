#ifndef LISTENER_H
#define LISTENER_H

#include <boost/asio.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <memory>

class Listener : public std::enable_shared_from_this<Listener> {
  boost::asio::io_context& ioc_;
  boost::asio::ssl::context& ctx_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::thread_pool& dbPool_;

public:
  Listener(
    boost::asio::io_context& ioc,
    boost::asio::ssl::context& ctx,
    boost::asio::ip::tcp::endpoint endpoint,
    boost::asio::thread_pool& dbPool
  );

  void run();

private:
  void doAccept();
  void onAccept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);
};
#endif
