# Secure Financial Gateway (continuous development)
This is a skill demo (low level cybersecurity programming). It handles sensitive financial operations while maintaining PCI DSS (Payment Card Industry Data Security Standard). I will continuously improve and strengthen it to uphold top-tier security practices and ensure the highest standards of privacy.

**Backend:** Boost.Beast (C++), JWT-CPP, OpenSSL, Argon2, CMake <br/>
**Frontend:** Vite + React TypeScript + Tailwind <br/>
**Database:** PostgreSQL

## Security Focus (Key Controls Implemented)
- **TLS 1.3 only** + manually disabling legacy protocols
- **PCI DSS** (Payment Card Industry Data Security Standard)
- **JWT validation middleware** - full claim verification using Bearer tokens
- **Prepared statements** for every database operation (libpqxx) - preventing SQL injection.
- **Rate limiting** on API endpoints (Asio timer)
- **Audit logging** for key actions (metadata only, no sensitive data)
- **Strict CSP** and input sanitization on React side
- **Axios interceptors** for automatic token handling and refresh
- **Login system** where passward is stored as hash (Argon2)

## TODO LIST
- **OAuth2** Resource Server
- **RS256** instead of HS256

> [!NOTE]
> Everything is ran locally (localhost) inside an Alpine Linux Podman container.
> Also, there's no `./build.sh` provided to get you started. I use Vim as my primary editor so the repository contains no IDE-specific configuration files. Simply clone the repository and open it with your preferred editor on GNU/Linux (will probably compile fine on Windows and MacOS, but it's untested).

## Required (TODO)
postgresql16-dev, boost-dev, openssl-dev, cmake, make, g++, linux-headers

## For Alpine Linux container (Podman)
If you're using the latest Alpine Linux container, you won't have libpqxx and will have to build it from source. Follow my instructions.

```
# Download libpqxx
cd /tmp
wget https://github.com/jtv/libpqxx/archive/refs/tags/8.0.0.tar.gz -O libpqxx-8.0.0.tar.gz

tar -xzf libpqxx-8.0.0.tar.gz
cd libpqxx-8.0.0

# Build and install
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
make install
```

## TODO JWT Manuage installation guide

## Server setup instructions (TODO)
```
export SFG_JWT_SECRET=tmp123
export SFG_JWT_ISSUER=sfg-gateway
export SFG_JWT_AUDIENCE=sfg-gateway-api
export DB_USER=sfg_user
export DB_PASSWORD=tmp123
```
