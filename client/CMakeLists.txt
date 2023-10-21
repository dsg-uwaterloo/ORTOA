
# Create the executable target
add_executable(client 
    client.cpp
    client_utils.cpp
    ../crypto/encryption_engine.cpp
    ../gen-cpp/RPC.cpp
    ../gen-cpp/Operation_types.cpp
)

target_include_directories(
  client
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}) # Needed for #include "../constants/constants.h"

# Link required libraries
target_link_libraries(client PRIVATE
    thrift
    sodium
    crypto
    ssl
    spdlog::spdlog
)
