set(CLIENT_EXECUTABLE client)

add_executable(${CLIENT_EXECUTABLE} 
    client.cpp
    client_utils.cpp
    ${CMAKE_BINARY_DIR}/src/gen-cpp/RPC.cpp
    ${CMAKE_BINARY_DIR}/src/gen-cpp/Operation_types.cpp
)

target_include_directories(${CLIENT_EXECUTABLE}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/src/gen-cpp
)

add_dependencies(${CLIENT_EXECUTABLE} generate-thrift)

target_link_libraries(${CLIENT_EXECUTABLE} 
    spdlog::spdlog
    thrift
    sodium
    pthread
    argparse
    libstorage
    libcommon
)

install(TARGETS client
    DESTINATION bin
)
