#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <cstdlib>

namespace beast = boost::beast;
namespace http = beast::http;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

// Environmental variables
std::string getEnvVal(const std::string& var, const std::string& def = "") {
  const char* val = std::getenv(var.c_str());
  return val ? val : def;
}

int main() {
  try {
    // DB Connection (localhost only for demo purposes)
    std::string connStr = "host=127.0.0.1 port=5432 dbname=sfg_db user=" + 
      getEnvVal("DB_USER", "") + 
      " password=" + getEnvVal("DB_PASSWORD", "");

    pqxx::connection dbConn(connStr);
    std::cout << "Connected to PostgreSQL: " << dbConn.dbname() << std::endl;

    // Test 
    pqxx::work txn(dbConn);
    auto result = txn.exec("SELECT COUNT(*) FROM transactions");
    std::cout << "Total transactions in DB: " << result[0][0].as<int>() << std::endl;
    txn.commit();

    // TODO: Add TLS 1.3 here later
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
