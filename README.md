# Secure Financial Gateway
This is a skill demo (cybersecurity programming). It handles sensitive financial operations while maintaining strict privacy and protection standards. I will continuously update and harden it to maintain the best security and the highest privacy standards.

**Backend:** Boost.Beast (C++), JWT-CPP, Argon2, CMake

**Frontend:** Vite + React TypeScript + Tailwind

**Database:** PostgreSQL

## Security Focus (Key Controls Implemented)
- **TLS 1.3 only** + manually disabling legacy protocols
- **JWT validation middleware** - full claim verification using Bearer tokens
- **Prepared statements** for every database operation (libpqxx) - preventing SQL injection.
- **Rate limiting** on API endpoints (Asio timer)
- **Audit logging** for key actions (metadata only, no sensitive data)
- **OAuth2 / OIDC** with PKCE flow on frontend (no refresh tokens in localStorage - memory preferred)
- **Strict CSP** and input sanitization on React side
- **Axios interceptors** for automatic token handling and refresh
- **Login system** where passward is stored as hash (Argon2)

> [!NOTE]
> Everything is ran locally (localhost) inside an Alpine Linux Podman container.
> Also, there's no `./build.sh` provided to get you started. I use Vim as my primary editor so the repository contains no IDE-specific configuration files. Simply clone the repository and open it with your preferred editor on GNU/Linux (might work on Windows and MacOS but it's untested).

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
