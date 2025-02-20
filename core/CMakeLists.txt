project(userver-core CXX)

include(GetUserverVersion)
message(STATUS "Userver version ${USERVER_VERSION}")

set(USERVER_NAMESPACE "userver" CACHE STRING "C++ namespace to use")
if (NOT "${USERVER_NAMESPACE}" STREQUAL "")
    set(USERVER_NAMESPACE_BEGIN "namespace ${USERVER_NAMESPACE} {" CACHE STRING "Open C++ namespace to use")
    set(USERVER_NAMESPACE_END "}" CACHE STRING "Close C++ namespace to use")
endif()

include(RequireDWCAS)
include(CheckFunctionExists)
check_function_exists("accept4" HAVE_ACCEPT4)
check_function_exists("pipe2" HAVE_PIPE2)

set(BUILD_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/build_config.hpp)
if(${CMAKE_SOURCE_DIR}/.git/HEAD IS_NEWER_THAN ${BUILD_CONFIG})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/build_config.hpp.in
    ${BUILD_CONFIG}
    ESCAPE_QUOTES @ONLY)
endif()

file(GLOB_RECURSE SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/*pp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*pp
  ${CMAKE_CURRENT_SOURCE_DIR}/internal/*pp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*pp
)

file(GLOB_RECURSE UNIT_TEST_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/*_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp
)

file(GLOB_RECURSE LIBUTEST_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/testing/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/testing/*.hpp
)
list(REMOVE_ITEM LIBUTEST_SOURCES ${UNIT_TEST_SOURCES})

list(REMOVE_ITEM SOURCES ${UNIT_TEST_SOURCES})

file(GLOB_RECURSE BENCH_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/*_benchmark.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*_benchmark.cpp
)
file(GLOB_RECURSE LIBUBENCH_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core_benchmark.cpp
)
list (REMOVE_ITEM SOURCES ${BENCH_SOURCES} ${LIBUBENCH_SOURCES})

file(GLOB_RECURSE INTERNAL_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/internal/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/internal/*.hpp
)

list (REMOVE_ITEM SOURCES ${INTERNAL_SOURCES})

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS
    program_options
    filesystem
    locale
    regex
    iostreams
)
find_package_required(ZLIB "zlib1g-dev")

if (USERVER_FEATURE_UTEST)
    if (USERVER_CONAN)
        find_package(benchmark REQUIRED)
        find_package(GTest REQUIRED)
    else()
        include(SetupGTest)
        include(SetupGBench)
    endif()
endif()

option(USERVER_FEATURE_SPDLOG_TCP_SINK "Use tcp_sink.h of the spdlog library for testing logs" ON)
if (NOT USERVER_FEATURE_SPDLOG_TCP_SINK)
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/component.cpp
    PROPERTIES COMPILE_FLAGS -DUSERVER_FEATURE_NO_SPDLOG_TCP_SINK=1
  )
endif()

find_package(Iconv REQUIRED)
find_package_required(OpenSSL "libssl-dev")

if (USERVER_CONAN)
    find_package(c-ares REQUIRED)
    find_package(CURL REQUIRED)
    find_package(cryptopp REQUIRED)
    find_package(yaml-cpp REQUIRED)
    find_package(fmt REQUIRED)
    find_package(spdlog REQUIRED)
    find_package(cctz REQUIRED)
    find_package(http_parser REQUIRED)
    find_package(libev REQUIRED)

    find_package(RapidJSON REQUIRED)
    target_compile_definitions(RapidJSON::RapidJSON INTERFACE RAPIDJSON_HAS_STDSTRING)

    find_package(concurrentqueue REQUIRED)
else()
    include(SetupCAres)
    include(SetupCURL)
    include(SetupCryptoPP)
    find_package_required(libyamlcpp "libyaml-cpp-dev")
    include(SetupFmt)
    include(SetupSpdlog)
    include(SetupCCTZ)
    find_package_required(Http_Parser "libhttp-parser-dev")
    find_package_required(LibEv "libev-dev")
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    SPDLOG_PREVENT_CHILD_FD
    SPDLOG_FMT_EXTERNAL
    [[SPDLOG_LEVEL_NAMES={"TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "OFF" }]]
    CRYPTOPP_ENABLE_NAMESPACE_WEAK=1
)

# https://github.com/jemalloc/jemalloc/issues/820
if (USERVER_FEATURE_JEMALLOC AND NOT USERVER_SANITIZE AND NOT MACOS)
  if (USERVER_CONAN)
    find_package(jemalloc REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC jemalloc::jemalloc)
  elseif (USERVER_OPEN_SOURCE_BUILD)
    find_package_required(Jemalloc "libjemalloc-dev")
    target_link_libraries(${PROJECT_NAME} PUBLIC Jemalloc)
  else()
    find_package_required(Jemalloc "libyandex-taxi-jemalloc-dev")
    target_link_libraries(${PROJECT_NAME} PUBLIC Jemalloc)
  endif()

  target_compile_definitions(${PROJECT_NAME} PRIVATE JEMALLOC_ENABLED)
endif()

# https://bugs.llvm.org/show_bug.cgi?id=16404
if (USERVER_SANITIZE AND NOT CMAKE_BUILD_TYPE MATCHES "^Rel")
  target_link_libraries(${PROJECT_NAME} PUBLIC userver-compiler-rt-parts)
endif()

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Threads::Threads
    Boost::locale
    sanitize-target
    CURL::libcurl
  PRIVATE
    userver-uboost-coro
    Boost::filesystem
    Boost::program_options
    Boost::iostreams
    Boost::regex
    Iconv::Iconv
    OpenSSL::Crypto
    OpenSSL::SSL
    ZLIB::ZLIB
)
if (USERVER_CONAN)
    target_link_libraries(${PROJECT_NAME}
      PUBLIC
        yaml-cpp
        fmt::fmt
        c-ares::cares
        cctz::cctz
        concurrentqueue::concurrentqueue
        Boost::stacktrace
      PRIVATE
        cryptopp-static
        http_parser::http_parser
        libev::libev
        spdlog::spdlog
        RapidJSON::RapidJSON
    )
else()
    target_link_libraries(${PROJECT_NAME}
      PUBLIC
        libyamlcpp
        fmt
        c-ares
        cctz
        userver-stacktrace
      PRIVATE
        CryptoPP
        Http_Parser
        LibEv
        spdlog_header_only
    )

    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
        ${USERVER_THIRD_PARTY_DIRS}/rapidjson/include
    )
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
        ${USERVER_THIRD_PARTY_DIRS}/moodycamel/include
    )
endif()

if (NOT MACOS)
  target_link_libraries(${PROJECT_NAME} PUBLIC atomic)
endif()

set(USERVER_PUBLIC_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include
  ${USERVER_THIRD_PARTY_DIRS}/pfr/include
)
set(USERVER_GLIBC_INCLUDE_FIXES_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/libc_include_fixes
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${USERVER_PUBLIC_INCLUDE_DIRS}
)
target_include_directories(${PROJECT_NAME} SYSTEM BEFORE PUBLIC
  ${USERVER_GLIBC_INCLUDE_FIXES_DIR}
)

# The bug is only triggered with optimizations enabled -- TAXICOMMON-1729
set_property(
  SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/errno_test.cpp
  APPEND PROPERTY COMPILE_FLAGS -O2
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${CMAKE_CURRENT_BINARY_DIR}
)

file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/" SRC_LOG_PATH_BASE)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/" BIN_LOG_PATH_BASE)
target_compile_definitions(${PROJECT_NAME} PUBLIC
  USERVER_LOG_SOURCE_PATH_BASE=${SRC_LOG_PATH_BASE}
  USERVER_LOG_BUILD_PATH_BASE=${BIN_LOG_PATH_BASE}
)

message(STATUS "Putting userver into namespace '${USERVER_NAMESPACE}': ${USERVER_NAMESPACE_BEGIN} ${USERVER_NAMESPACE_END}")
target_compile_definitions(${PROJECT_NAME} PUBLIC
  "USERVER_NAMESPACE=${USERVER_NAMESPACE}"
  "USERVER_NAMESPACE_BEGIN=${USERVER_NAMESPACE_BEGIN}"
  "USERVER_NAMESPACE_END=${USERVER_NAMESPACE_END}"
)

add_library(userver-core-internal STATIC ${INTERNAL_SOURCES})
target_compile_definitions(userver-core-internal PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME},COMPILE_DEFINITIONS>)
target_include_directories(userver-core-internal PUBLIC
  $<TARGET_PROPERTY:${PROJECT_NAME},INCLUDE_DIRECTORIES>
  ${CMAKE_CURRENT_SOURCE_DIR}/internal/include
)
target_link_libraries(userver-core-internal
  PUBLIC
    ${PROJECT_NAME}
)

if (USERVER_FEATURE_UTEST)
    add_library(userver-utest STATIC ${LIBUTEST_SOURCES})
    target_compile_definitions(userver-utest PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME},COMPILE_DEFINITIONS>)

    target_link_libraries(userver-utest
      PUBLIC
        ${PROJECT_NAME}
      PRIVATE
        userver-core-internal
        Boost::program_options
    )
    if (USERVER_CONAN)
        target_link_libraries(userver-utest PUBLIC GTest::gtest GTest::gmock)
    else()
        target_link_libraries(userver-utest PUBLIC libgtest libgmock)
    endif()

    target_include_directories(userver-utest PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/testing/include
    )
    target_include_directories(userver-utest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/testing/src/
    )

    # Target with no need to use userver namespace, but includes require userver/
    add_library(yandex-userver-utest INTERFACE)
    target_link_libraries(yandex-userver-utest INTERFACE userver-utest)
endif()

if (USERVER_IS_THE_ROOT_PROJECT)
    add_executable(${PROJECT_NAME}_unittest ${UNIT_TEST_SOURCES})
    target_include_directories (${PROJECT_NAME}_unittest PRIVATE
        $<TARGET_PROPERTY:${PROJECT_NAME},INCLUDE_DIRECTORIES>
    )
    target_link_libraries(${PROJECT_NAME}_unittest PRIVATE
      userver-utest
      userver-core-internal
    )

    # We keep testing deprecated functions, no need to warn about that
    target_compile_options(${PROJECT_NAME}_unittest PRIVATE "-Wno-deprecated-declarations")
    add_google_tests(${PROJECT_NAME}_unittest)
    add_subdirectory(functional_tests)
endif()

if (USERVER_FEATURE_UTEST)
    add_library(userver-ubench ${LIBUBENCH_SOURCES})
    target_include_directories(userver-ubench PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME},INCLUDE_DIRECTORIES>)
    target_compile_definitions(userver-ubench PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME},COMPILE_DEFINITIONS>)
    target_link_libraries(userver-ubench
      PUBLIC
        ${PROJECT_NAME}
      PRIVATE
        userver-core-internal
    )
    if (USERVER_CONAN)
        target_link_libraries(userver-ubench PUBLIC benchmark::benchmark)
    else()
        target_link_libraries(userver-ubench PUBLIC libbenchmark)
    endif()

    # Target with no need to use userver namespace, but includes require userver/
    add_library(yandex-userver-ubench INTERFACE)
    target_link_libraries(yandex-userver-ubench INTERFACE userver-ubench)
endif()

if (USERVER_IS_THE_ROOT_PROJECT)
    add_executable(${PROJECT_NAME}_benchmark ${BENCH_SOURCES})
    target_link_libraries(${PROJECT_NAME}_benchmark
      PUBLIC
        userver-ubench
      PRIVATE
        userver-core-internal
    )
    add_google_benchmark_tests(${PROJECT_NAME}_benchmark)
endif()

# Target with no need to use userver namespace, but includes require userver/
add_library(yandex-${PROJECT_NAME} INTERFACE)
target_link_libraries(yandex-${PROJECT_NAME} INTERFACE ${PROJECT_NAME})
