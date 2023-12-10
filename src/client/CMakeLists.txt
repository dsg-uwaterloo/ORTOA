
# Create the executable target
add_executable(client 
    client.cpp
    client_utils.cpp
    client_utils.h
    SharedQueue.h
    ../crypto/encryption_engine.cpp
    ../gen-cpp/RPC.h
    ../gen-cpp/RPC.cpp
    ../gen-cpp/Operation_types.h
    ../gen-cpp/Operation_types.cpp
)



add_dependencies(client generate-thrift)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/constants
    ${CMAKE_BINARY_DIR}/src/gen-cpp
)

# Link reuired libraries
target_link_libraries(client 
    ortoadb 
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
