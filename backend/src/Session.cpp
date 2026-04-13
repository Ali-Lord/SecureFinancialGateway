#include <gateway/Session.h>
#include <boost/json.hpp>
#include <pqxx/pqxx>
#include <argon2.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
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
    res->set("X-Content-Type-Options", "nosniff"); // I used string cuz my Boost version doesn't have this enum

    http::async_write(self->stream_, *res, [self, res](beast::error_code ec, std::size_t) {
      beast::error_code ignore;
      self->stream_.shutdown(ignore); // TODO: handle this
    });
  });
}

bool Session::validateJWT(const std::string& token, std::string& errorMsg){
  try {
    auto decoded = jwt::decode(token);

    // TODO: temporary HS256 algorithm for testing purposes, ofc. Will change to RS256 later.
    auto verifier = jwt::verify()
      .allow_algorithm(jwt::algorithm::hs256{getEnvVar("SFG_JWT_SECRET")})
      .with_issuer(getEnvVar("SFG_JWT_ISSUER"))
      .with_audience(getEnvVar("SFG_JWT_AUDIENCE"));

    verifier.verify(decoded);

    /*
     * NOTE: I manually built and installed jwt-cpp (v0.7.2) with CMake from source. My version
     * of jwt-cpp doesn't have get_payload_claims(), but only get_payload_claim()
    */
    if (decoded.has_payload_claim("scope")) {
      std::string scope = decoded.get_payload_claim("scope").as_string();
      if (scope != "transactions:read") {
        errorMsg = "insufficient_scope";
        return false;
      }
    }

    return true;

    // Standard way to check scopes if you did NOT build from source (jwt-cpp)
    // For jwt-cpp repo reference: https://github.com/Thalhammer/jwt-cpp (v0.7.2)
    /*
    auto payload = decoded.get_payload_claims();
    if (!payload.count("scope")) {
      errorMsg = "scope_not_found";
      return false;
    } else if (payload.at("scope").as_string() != "transactions:read") {
      errorMsg = "insufficient_scope";
      return false;
    }

    return true;
    */
  } catch (const std::exception& e) {
    errorMsg = e.what();
    return false;
  }
}

// ===========================================
// ========== PASSWORD VERIFICATION ==========
/*
 * NOTE: Unfortunately, my container's argon2-dev package does not include
 * high-level argon2id_hash_encoded / argon2id_encode_string functions. That
 * means, I gotta use raw API (argon2id_hash_raw) + OpenSSL for Base64.
*/
std::string Session::base64Encode(const uint8_t* input, size_t length){
  BIO* b64 = BIO_new(BIO_f_base64());
  BIO* bio = BIO_new(BIO_s_mem());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_push(b64, bio);

  BIO_write(bio, input, static_cast<int>(length));
  BIO_flush(bio);

  BUF_MEM* mem = nullptr;
  BIO_get_mem_ptr(bio, &mem);
  std::string encoded(mem->data, mem->length);

  BIO_free_all(bio);

  /*
   * Thhe standard Argon2 PHC format uses base64 without padding.
   * This OpenSSL encoder adds padding. Gotta remove the padding to Argon2 PHC format.
  */
  while (!encoded.empty() && encoded.back() == '=') {
    encoded.pop_back();
  }
  return encoded;
}

std::string Session::hashPassword(const std::string& password) {
  const uint32_t tCost = 3;
  const uint32_t mCost = 65536;
  const uint32_t parallelism = 4;
  const uint32_t hashLen = 32;

  std::vector<uint8_t> hash(hashLen);
  std::vector<uint8_t> salt = {'s','f','g','_','s','a','l','t'}; // TODO: randomly generate for every single user

  int ret = argon2id_hash_raw(
      tCost,
      mCost,
      parallelism,
      password.data(),
      password.size(),
      salt.data(),
      salt.size(),
      hash.data(),
      hash.size()
    );

  if (ret != ARGON2_OK) {
    throw std::runtime_error("argon2 hash failed");
  }

  std::string b64Salt = base64Encode(salt.data(), salt.size());
  std::string b64Hash = base64Encode(hash.data(), hash.size());

  // Standard PHC encoded string
  return "$argon2id$v=19$m=" + std::to_string(mCost) +
    ",t=" + std::to_string(tCost) +
    ",p=" + std::to_string(parallelism) +
    "$" + b64Salt + "$" + b64Hash;
}

bool Session::verifyPassword(const std::string& password, const std::string& encodedHash) {
  int ret = argon2_verify(encodedHash.c_str(), password.c_str(), password.length(), Argon2_id);
  return ret == ARGON2_OK;
}

// ==========================================
// ========== RESTful API HANDLERS ==========
void Session::handleRequest() {
  std::string target = std::string(req_.target());
  std::string method = std::string(req_.method_string());

  // Login (public)
  if (method == "POST" && target == "/api/login") {
    boost::json::value result = handleLogin(req_.body());
    sendResponse(http::status::ok, boost::json::serialize(result));
    return;
  }

  // Early return on failure to prevent garbage input (save CPU resources)
  std::string auth = std::string(req_[http::field::authorization]); // TODO: include wwww_authenticate
  if (!auth.starts_with("Bearer ")) {
    sendResponse(http::status::unauthorized, "missing_token_error");
    return;
  }

  // JWT validation
  std::string token = auth.substr(7);
  std::string jwtError;
  if (!validateJWT(token, jwtError)) {
      sendResponse(http::status::unauthorized, jwtError);

      return;
  }

  // Protected endpoints
  if (method == "GET" && target == "/api/status") {
    boost::json::value result = handleStatus();
    sendResponse(http::status::ok, boost::json::serialize(result));
  }
  else if (method == "GET" && target == "/api/transactions") {
    boost::json::value result = handleTransactions();
    sendResponse(http::status::ok, boost::json::serialize(result));
  }
  else {
      sendResponse(http::status::not_found, "not_found");
  }
}

boost::json::value Session::handleLogin(const std::string& body) {
  try {
    auto json = boost::json::parse(body);
    std::string email = json.at("email").as_string().c_str();
    std::string password = json.at("password").as_string().c_str();

    pqxx::connection db("host=127.0.0.1 port=5432 dbname=sfg_db "
        "user=" + getEnvVar("DB_USER") + " password=" + getEnvVar("DB_PASSWORD"));

    pqxx::work txn(db);

    auto r = txn.exec_params("SELECT id, password_hash, role FROM users WHERE email = $1", email);
    if (r.empty()) {
      return boost::json::object{{"error", "invalid_credentials"}};
    }

    if (!verifyPassword(password, r[0][1].c_str())) {
      // Log failed attempts
      txn.exec_params("INSERT INTO audit_log (user_id, action, ip_address, details) "
          "VALUES ($1, 'login_failed', '127.0.0.1', $2)",
          r[0][0].as<int>(), "{\"reason\": \"wrong_password\"}");
      txn.commit();

      return boost::json::object{{"error", "invalid_credentials"}};
    }

    auto token = jwt::create()
      .set_issuer(getEnvVar("SFG_JWT_ISSUER", "sfg-gateway"))
      .set_audience(getEnvVar("SFG_JWT_AUDIENCE", "sfg-gateway-api"))
      .set_issued_at(std::chrono::system_clock::now())
      .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(3600))
      .set_payload_claim("user_id", jwt::claim(std::to_string(r[0][0].as<int>())))
      .set_payload_claim("role", jwt::claim(std::string(r[0][2].c_str())))
      .set_payload_claim("scope", jwt::claim(std::string("transactions:read")))
      .sign(jwt::algorithm::hs256{getEnvVar("SFG_JWT_SECRET")});

    // Log successful attempts
    txn.exec_params("INSERT INTO audit_log (user_id, action, ip_address, details) "
        "VALUES ($1, 'login_success', '127.0.0.1', $2)",
        r[0][0].as<int>(), "{\"success\": true}");
    txn.commit();

    boost::json::object res;
    res["token"] = token;
    res["role"] = r[0][2].c_str();
    return res;
  } catch (const std::exception& e) {
    return boost::json::object{{"error", "invalid_request"}}; // TODO: handle the exceptions
  }
}

boost::json::value Session::handleStatus() {
    try {
        std::string connStr = "host=127.0.0.1 port=5432 dbname=sfg_db "
          "user=" + getEnvVar("DB_USER") + " password=" + getEnvVar("DB_PASSWORD");
        pqxx::connection db(connStr);
        pqxx::work txn(db);

        auto totalVol = txn.exec("SELECT COALESCE(SUM(amount), 0) FROM transactions WHERE status = 'success'");
        auto successCount = txn.exec("SELECT COUNT(*) FROM transactions WHERE status = 'success'");
        auto totalCount = txn.exec("SELECT COUNT(*) FROM transactions");
        auto recent = txn.exec("SELECT txn_id, amount, currency, status, created_at FROM transactions ORDER BY created_at DESC LIMIT 5");

        boost::json::object obj;
        obj["total_volume_hkd"] = totalVol[0][0].as<double>();
        obj["success_rate_pct"] = (totalCount[0][0].as<int>() == 0) ? 0.0 :
                                  (successCount[0][0].as<double>() / totalCount[0][0].as<double>() * 100);
        obj["total_transactions"] = totalCount[0][0].as<int>();

        boost::json::array recentArr;
        for (auto row : recent) {
            boost::json::object t;
            t["txn_id"] = row["txn_id"].c_str();
            t["amount"] = row["amount"].as<double>();
            t["currency"] = row["currency"].c_str();
            t["status"] = row["status"].c_str();
            t["created_at"] = row["created_at"].c_str();
            recentArr.push_back(std::move(t));
        }
        obj["recent_transactions"] = std::move(recentArr);

        return obj;
    } catch (const std::exception& e) {
      std::cerr << "[STATUS ERROR] " << e.what() << std::endl;
      return boost::json::object{{"error", "database_error"}}; // TODO: Handle exceptions properly
    }
}

boost::json::value Session::handleTransactions() {
    try {
        pqxx::connection db("host=127.0.0.1 port=5432 dbname=sfg_db "
                            "user=" + getEnvVar("DB_USER") + " password=" + getEnvVar("DB_PASSWORD"));
        pqxx::work txn(db);
        auto r = txn.exec("SELECT txn_id, amount, currency, status, processor_ref, created_at "
                          "FROM transactions ORDER BY created_at DESC LIMIT 20");

        boost::json::array arr;
        for (auto row : r) {
            boost::json::object t;
            t["txn_id"] = row["txn_id"].c_str();
            t["amount"] = row["amount"].as<double>();
            t["currency"] = row["currency"].c_str();
            t["status"] = row["status"].c_str();
            t["processor_ref"] = row["processor_ref"].c_str();
            t["created_at"] = row["created_at"].c_str();
            arr.push_back(std::move(t));
        }
        return arr;
    } catch (const std::exception& e) {
        return boost::json::array{}; // TODO: handle exceptions properly.
    }
}

std::string Session::getEnvVar(const std::string& var, const std::string& def) {
  const char* val = std::getenv(var.c_str());
  return val ? val : def;
}
