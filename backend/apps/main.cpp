#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <pqxx/pqxx>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

// Environmental variable
std::string getEnvVal(const std::string& var, const std::string& def = "") {
  const char* val = std::getenv(var.c_str());
  return val ? val : def;
}

// Temporary for the sake of rapid prottotyping, I'll have Session class in the same file as main.cpp
// TODO: Seperate the files
class Session : public std::enable_shared_from_this<Session> {
  ssl::stream<beast::tcp_stream> stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  boost::asio::thread_pool& dbPool_;

public:
  explicit Session(tcp::socket&& socket, ssl::context& ctx, boost::asio::thread_pool& dbPool)
    : stream_(std::move(socket), ctx), dbPool_(dbPool) {}

  void run() {
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
    stream_.async_handshake(
        ssl::stream_base::server,
        beast::bind_front_handler(&Session::onHandshake, shared_from_this())
    );
  }

private:
  void onHandshake(beast::error_code ec) {
    if (!ec) {
      doRead();
    }
    else {
      std::cerr << "[TLS] Handshake failed: " << ec.message() << std::endl;
      return;
    }
  }

  void doRead() {
    req_ = {};
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
    http::async_read(
        stream_,
        buffer_,
        req_,
        beast::bind_front_handler(&Session::onRead, shared_from_this())
    );
  }

  void onRead(beast::error_code ec, std::size_t) {
    if (!ec) {
      handleRequest();
    }
    else if (ec == http::error::end_of_stream) {
      std::cerr << "[ERROR] End of stream: " << ec.message() << std::endl;
    }
    else {
      return;
    }
  }

  void handleRequest() {
    if (req_.method() == http::verb::get && req_.target() == "/api/status") {
      boost::asio::post(dbPool_, [self = shared_from_this()]() {
        try {
          // DB Connection (localhost only for demo purposes)
          std::string connStr = "host=127.0.0.1 port=5432 dbname=sfg_db user=" +
            getEnvVal("DB_USER") + " password=" + getEnvVal("DB_PASSWORD");

          pqxx::connection db(connStr);
          pqxx::work txn(db);
          
          auto r = txn.exec("SELECT COUNT(*) FROM transactions");
          int count = r[0][0].as<int>();

          boost::json::object obj;
          obj["status"] = "ok";
          obj["transactions_count"] = count;
          obj["timestamp"] = std::time(nullptr);

          self->sendResponse(http::status::ok, boost::json::serialize(obj));
        } catch (const pqxx::sql_error& ec) {
          std::cerr << "[SQL_ERROR] " << ec.what() << " Query: " << ec.query() << std::endl;
          self->sendResponse(http::status::internal_server_error, "database_query_error");

        } catch (const std::exception& ec) {
          std::cerr << "[DB_ERROR] " << ec.what() << std::endl;
          self->sendResponse(http::status::internal_server_error, "database_error");
        }
      });
    } else {
      sendResponse(http::status::not_found, "not_found");
    }
  }

  void sendResponse(http::status status, std::string body) {
    boost::asio::dispatch(stream_.get_executor(), [self = shared_from_this(), status, body] {
      auto res = std::make_shared<http::response<http::string_body>>(status, self->req_.version());
      res->set(http::field::content_type, "application/json");
      res->body() = body;
      res->prepare_payload();
      res->version(self->req_.version());
      res->keep_alive(false);
      res->set(http::field::server, "SecureFinancialGateway");

      // Security headers
      res->set(http::field::strict_transport_security, "max-age=31536000; includeSubdomains");
      res->set(http::field::x_frame_options, "DENY");
      res->set("X-Content-Type-Options", "nosniff"); // I used string name cuz my Boost version doesn't have this enum

      http::async_write(self->stream_, *res, [self, res](beast::error_code ec, std::size_t) {
        beast::error_code ignore;
        self->stream_.shutdown(ignore); // TODO: handle this
      });
    });
  }
};

class Listener : public std::enable_shared_from_this<Listener> {
  boost::asio::io_context& ioc_;
  ssl::context& ctx_;
  tcp::acceptor acceptor_;
  boost::asio::thread_pool& dbPool_;

public:
  Listener(boost::asio::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, boost::asio::thread_pool& dbPool)
    :ioc_(ioc), ctx_(ctx), acceptor_({ioc, endpoint}), dbPool_(dbPool) {}

  void run() { doAccept(); };

private:
  void doAccept() {
    acceptor_.async_accept(boost::asio::make_strand(ioc_),
        beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
  }

  void onAccept(beast::error_code ec, tcp::socket socket) {
    if (!ec) {
      std::make_shared<Session>(std::move(socket), ctx_, dbPool_)->run();
    }
    doAccept();
  }
};

int main() {
  try {
    boost::asio::io_context ioc{1};
    boost::asio::thread_pool dbPool{4};

    // TLS 1.3 only (I went further by even manually disabled others)
    ssl::context ctx{ssl::context::tlsv13_server};
    ctx.set_options(
      ssl::context::default_workarounds |
      ssl::context::no_sslv2 |
      ssl::context::no_sslv3 |
      ssl::context::no_tlsv1 |
      ssl::context::no_tlsv1_1 |
      ssl::context::no_tlsv1_2
    );
    ctx.use_certificate_chain_file("cert.pem"); // I'll create self-signed instead (for demo purposes, ofc)
    ctx.use_private_key_file("key.pem", ssl::context::pem);

    std::make_shared<Listener>(ioc, ctx, tcp::endpoint{{}, 8443}, dbPool)->run();
    std::cout << "SecureFinacialGateway listening on https://localhost:8443 (TLS 1.3 only)" << std::endl;
    ioc.run();
  } catch (const std::exception& ec) {
    std::cerr << "[CRITICAL STARTUP ERROR]: " << ec.what() << std::endl;
    return 1;
  }

  return 0;
}
