cmake_minimum_required(VERSION 3.20)
project(SecureFinancialGateway)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system json)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

add_executable(SecureFinacialgateway backend/src/main.cpp)

target_link_libraries(SecureFinancialGateway Boost::system Boost::json ${PQXX_LIBRARIES})
target_include_directories(SecureFinacialgateway PRIVATE ${PQXX_INCLUDE_DIRS} backend/include)
