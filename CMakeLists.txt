cmake_minimum_required(VERSION 2.8)
project (bookie-cpp)

set(CMAKE_CXX_FLAGS "-std=c++14 -g0 -O3 -march=native -pthread")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

find_package(Folly REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_library(GLOG_LIBRARY_PATH glog)
find_library(GFLAGS_LIBRARY_PATH gflags)

find_package(ZooKeeper REQUIRED)
find_library(LOG4CXX_LIBRARY_PATH log4cxx)
find_library(ROCKSDB_LIBRARY_PATH rocksdb)
find_library(WANGLE_LIBRARY_PATH wangle)
find_library(JEMALLOC_LIBRARY_PATH jemalloc)

include_directories(
  ${CMAKE_SOURCE_DIR}/..
  ${FOLLY_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${INCLUDE_DIR}
)

set(BOOKIE_SOURCES
  src/Bookie.cpp
  src/BookieCodecV2.cpp
  src/BookieConfig.cpp
  src/BookieHandler.cpp
  src/BookiePipeline.cpp
  src/BookieProtocol.cpp
  src/BookieRegistration.cpp
  src/Logging.cpp
  src/Storage.cpp
  src/ZooKeeper.cpp
  src/main.cpp
)

add_executable(bookie ${BOOKIE_SOURCES})

target_link_libraries(bookie
  ${FOLLY_LIBRARIES}
  ${LOG4CXX_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${GLOG_LIBRARY_PATH}
  ${GFLAGS_LIBRARY_PATH}
  ${ROCKSDB_LIBRARY_PATH}
  ${LOG4CXX_LIBRARY_PATH}
  ${Zookeeper_LIBRARY}
  ${WANGLE_LIBRARY_PATH}
  ${JEMALLOC_LIBRARY_PATH}
)

if (NOT APPLE)
    target_link_libraries(bookie rt atomic)
endif()



