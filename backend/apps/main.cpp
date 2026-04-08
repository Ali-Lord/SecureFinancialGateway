#include <gateway/Listener.h>
#include <boost/asio.hpp>
#include <iostream>
#include <cstdlib>

int main() {
  try {
    boost::asio::io_context ioc{1};
    boost::asio::thread_pool dbPool{4};

    // TLS 1.3 only (I went further by even manually disabled others)
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv13_server};
    ctx.set_options(
      boost::asio::ssl::context::default_workarounds |
      boost::asio::ssl::context::no_sslv2 |
      boost::asio::ssl::context::no_sslv3 |
      boost::asio::ssl::context::no_tlsv1 |
      boost::asio::ssl::context::no_tlsv1_1 |
      boost::asio::ssl::context::no_tlsv1_2
    );
    ctx.use_certificate_chain_file("cert.pem"); // I'll create self-signed instead (for demo purposes, ofc)
    ctx.use_private_key_file("key.pem", boost::asio::ssl::context::pem);

    std::make_shared<Listener>(ioc, ctx, boost::asio::ip::tcp::endpoint{{}, 8443}, dbPool)->run();
    std::cout << "SecureFinacialGateway listening on https://localhost:8443 (TLS 1.3 only)" << std::endl;
    ioc.run();
  } catch (const std::exception& ec) {
    std::cerr << "[CRITICAL STARTUP ERROR]: " << ec.what() << std::endl;
    return 1;
  }

  return 0;
}
