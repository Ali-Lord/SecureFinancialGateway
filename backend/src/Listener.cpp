#include <gateway/Listener.h>
#include <gateway/Session.h>

namespace beast = boost::beast;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

Listener::Listener(boost::asio::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, boost::asio::thread_pool& dbPool)
  :ioc_(ioc), ctx_(ctx), acceptor_({ioc, endpoint}), dbPool_(dbPool) {}

void Listener::run() { doAccept(); };

void Listener::doAccept() {
  acceptor_.async_accept(boost::asio::make_strand(ioc_),
      beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
}

void Listener::onAccept(beast::error_code ec, tcp::socket socket) {
  if (!ec) {
    std::make_shared<Session>(std::move(socket), ctx_, dbPool_)->run();
  }
  doAccept();
}

