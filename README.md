# Secure Financial Gateway
This project showcases a secure financial gateway designed with security and reliability as core principles. It handles sensitive financial operations while maintaining strict privacy and protection standards. Originally I built this for my past job assessment, this can now be used as a project to learn how a system should be built if you're new to the world of cybersecurity (in the field of finance). I will continuously update and harden it to maintain the highest levels of robustness and security.

**Backend:** Boost.Beast (C++)  
**Frontend:** React TypeScript + Tailwind  
**Database:** PostgreSQL

## Security Focus (Key Controls Implemented)
- **TLS 1.3 only** with strict security headers on all HTTPS endpoints
- **JWT validation middleware** - full claim verification using Bearer tokens
- **Prepared statements** for every database operation (libpqxx) - preventing SQL injection.
- **Rate limiting** on API endpoints (Asio timer)
- **Audit logging** for key actions (metadata only, no sensitive data)
- **OAuth2 / OIDC** with PKCE flow on frontend (no refresh tokens in localStorage - memory preferred)
- **Strict CSP** and input sanitization on React side
- **Axios interceptors** for automatic token handling and refresh

**Note:** This is a demonstration project. Always follow proper security practices and never use demo credentials or configurations in production environments. Everything is ran locally (localhost) inside an Alpine Linux Podman container. If you're working on a free open-source project and need guidance on security best practices, feel free to reach out.

## Architecture
**Backend**
- Async HTTPS server (Boost.Beast)
- CMake + vcpkg/conan dependency management
- Protected endpoints:
  - `POST /api/transaction` - JWT + input validation, insert transaction
  - `GET /api/transactions` - user-specific list
  - `GET /api/status` - summary statistics

**Frontend**
- Vite + React TS + Tailwind (TailAdmin template)
- Dashboard with transaction overview, charts (Recharts/Chart.js), and filterable tables
- Admin security section (active sessions, audit logs)
- CSV export (client-side only)

## Required
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
