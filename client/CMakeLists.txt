
# Create the executable target
add_executable(client 
    client.cpp
    ../host/redis.cpp 
    ../crypto/encryption_engine.cpp
)

# Link reuired libraries
target_link_libraries(client PRIVATE
    /usr/local/lib/libredis++.a
    /usr/local/lib/libhiredis.a
    crypto
    ssl
)
