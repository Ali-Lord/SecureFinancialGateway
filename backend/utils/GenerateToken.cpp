#include <jwt-cpp/jwt.h>
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]) {
  if (argc !=4) {
    std::cerr << "Incorrect number of arguments detected!\n\n"
      "Usage:\n./GenerateToken [SFG_JWT_SECRET] [SFG_JWT_ISSUER] [JWT_AUDIENCE]" << std::endl;

    return 1;
  }

  std::string secret = argv[1];
  std::string issuer = argv[2];
  std::string audience = argv[3];

  auto token = jwt::create()
    .set_issuer(issuer)
    .set_audience(audience)
    .set_issued_at(std::chrono::system_clock::now())
    .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(3600))
    .set_payload_claim("scope", jwt::claim(std::string("transactions:read")))
    .sign(jwt::algorithm::hs256{secret});

  std::cout << "Copy the token below:\n" << token << std::endl;

}
