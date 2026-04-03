#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;


// To test if everything's working properly
int main(int argc, char** argv){
  if (argc != 4){
    std::cerr << "Usage: " << argv[0] << " <host> <port> <target>\n";
    return 1;
  }

  auto const host = argv[1];
  auto const port = argv[2];
  auto const target = argv[3];

  asio::io_context ioc;
  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);

  auto const results = resolver.resolve(host, port);
  stream.connect(results);

  http::request<http::string_body> req{http::verb::get, target,  11};
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  http::write(stream, req);

  beast::flat_buffer buffer;
  http::response<http::dynamic_body> res;

  http::read(stream, buffer, res);

  std::cout << res << std::endl;

  beast::error_code ec;
  stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  return 0;
}
