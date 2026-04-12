cmake_minimum_required(VERSION 3.22)
project(SecureFinancialGateway LANGUAGES CXX)

set(CMAKE_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS system json)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(ARGON2 REQUIRED libargon2)

add_executable(SecureFinancialGateway
  apps/main.cpp
  src/Session.cpp
  src/Listener.cpp
)

target_include_directories(SecureFinancialGateway PRIVATE
    include
    ${PQXX_INCLUDE_DIRS}
    ${ARGON2_INCLUDE_DIRS}
)

target_link_libraries(SecureFinancialGateway
    Boost::system
    Boost::json
    ${PQXX_LIBRARIES}
    pq
    ssl
    crypto
    ${ARGON2_LIBRARIES}
)

message(STATUS "Boost include: ${Boost_INCLUDE_DIRS}")
message(STATUS "libpqxx include: ${PQXX_INCLUDE_DIRS}")
message(STATUS "OpenSSL libs: ssl + crypto")
