#include <gateway/Session.h>
#include <boost/json.hpp>
#include <pqxx/pqxx>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

Session::Session(
  boost::asio::ip::tcp::socket&& socket,
  boost::asio::ssl::context& ctx,
  boost::asio::thread_pool& dbPool)
  : stream_(std::move(socket), ctx), dbPool_(dbPool){}

void Session::run() {
  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
  stream_.async_handshake(
      ssl::stream_base::server,
      beast::bind_front_handler(&Session::onHandshake, shared_from_this())
  );
}

void Session::onHandshake(beast::error_code ec) {
  if (!ec) {
    doRead();
  }
  else {
    std::cerr << "[TLS] Handshake failed: " << ec.message() << std::endl;
    return;
  }
}

void Session::doRead() {
  req_ = {};
  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
  http::async_read(
      stream_,
      buffer_,
      req_,
      beast::bind_front_handler(&Session::onRead, shared_from_this())
  );
}

void Session::onRead(beast::error_code ec, std::size_t) {
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

void Session::handleRequest() {
  if (req_.method() == http::verb::get && req_.target() == "/api/status") {
    boost::asio::post(dbPool_, [self = shared_from_this()]() {
      try {
        // DB Connection (localhost only for demo purposes)
        std::string connStr = "host=127.0.0.1 port=5432 dbname=sfg_db user=" +
          self->getEnvVar("DB_USER") + " password=" + self->getEnvVar("DB_PASSWORD");

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

void Session::sendResponse(http::status status, std::string body) {
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

std::string Session::getEnvVar(const std::string& var, const std::string& def) {
  const char* val = std::getenv(var.c_str());
  return val ? val : def;
}

