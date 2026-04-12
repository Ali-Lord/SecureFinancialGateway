#include <argon2.h>
#include <iostream>
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

std::string base64Encode(const uint8_t* input, size_t length){
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

  while (!encoded.empty() && encoded.back() == '=') {
    encoded.pop_back();
  }

  return encoded;
}

std::string hashPassword(const std::string& password) {
  const uint32_t tCost = 3;
  const uint32_t mCost = 65536;
  const uint32_t parallelism =4;
  const uint32_t hashLen = 32;

  std::vector<uint8_t> hash(hashLen);
  std::vector<uint8_t> salt = {'s','f','g','_','s','a','l','t'}; // TODO: generate a random salt for evrey user

  int ret = argon2id_hash_raw(
      tCost,
      mCost,
      parallelism,
      password.data(),
      password.size(),
      salt.data(),
      salt.size(),
      hash.data(),
      hashLen
  );

  if (ret != ARGON2_OK) {
    std::cerr << "Hashing failed" << std::endl;
  }

  std::string b64Salt = base64Encode(salt.data(), salt.size());
  std::string b64Hash = base64Encode(hash.data(), hash.size());

  // PHC String
  return "$argon2id$v=19$m=" + std::to_string(mCost) +
  ",t=" + std::to_string(tCost) +
  ",p=" + std::to_string(parallelism) +
  "$" + b64Salt + "$" + b64Hash;
}

int main() {
  std::string pw;
  std::cout <<"Enter password to has: ";
  std::getline(std::cin, pw);

  std::string hash = hashPassword(pw);
  if (!hash.empty()) {
    std::cout << "Argon2id Hash:\n" << hash << std::endl;
  }

  return 0;
}
