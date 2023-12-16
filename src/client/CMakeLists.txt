
# Create the executable target
add_executable(client 
    client.cpp
    client_utils.cpp
    client_utils.h
    SharedQueue.h
    ../host/redis.cpp 
    ../crypto/encryption_engine.cpp
    ../gen-cpp/RPC.h
    ../gen-cpp/RPC.cpp
    ../gen-cpp/Operation_types.h
    ../gen-cpp/Operation_types.cpp
)

# fetch latest argparse
include(FetchContent)
set(ARGPARSE_BUILD_TESTS OFF CACHE INTERNAL "Turn off building argparse tests")
set(ARGPARSE_BUILD_SAMPLES OFF CACHE INTERNAL "Turn off building argparse samples")
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

add_dependencies(client generate-thrift)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/constants
    ${CMAKE_BINARY_DIR}/src/gen-cpp
)

# Link reuired libraries
target_link_libraries(client PRIVATE
    /usr/local/lib/libredis++.a
    /usr/local/lib/libhiredis.a
    crypto
    ssl
    spdlog::spdlog
    thrift
    sodium
    pthread
    argparse
)

install(TARGETS client
    DESTINATION bin
)
