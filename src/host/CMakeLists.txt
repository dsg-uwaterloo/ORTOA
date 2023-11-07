
add_custom_command(
  OUTPUT ortoa_u.h ortoa_u.c ortoa_args.h
  DEPENDS ${CMAKE_SOURCE_DIR}/src/ortoa.edl
  COMMAND
    openenclave::oeedger8r --untrusted ${CMAKE_SOURCE_DIR}/src/ortoa.edl
    --search-path ${OE_INCLUDEDIR} 
    --search-path ${OE_INCLUDEDIR}/openenclave/edl/sgx)


add_executable(ortoa-host
               host.cpp 
               redis.h
               redis.cpp
               ../constants/constants.h
               ${CMAKE_BINARY_DIR}/src/gen-cpp/RPC.h
               ${CMAKE_BINARY_DIR}/src/gen-cpp/RPC.cpp 
               ${CMAKE_BINARY_DIR}/src/gen-cpp/Operation_types.h
               ${CMAKE_BINARY_DIR}/src/gen-cpp/Operation_types.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/ortoa_u.c
)

add_dependencies(ortoa-host generate-thrift)

target_include_directories(
  ortoa-host
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} # Needed for #include "../shared.h"
          ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_BINARY_DIR}/src/gen-cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

target_link_libraries(ortoa-host 
    openenclave::oehost 
    rt
    spdlog::spdlog
    boost_filesystem
    boost_serialization
    thrift
    sodium
    pthread
)

# <------------ add hiredis dependency --------------->
find_path(HIREDIS_HEADER hiredis)
target_include_directories(ortoa-host PUBLIC ${HIREDIS_HEADER})

find_library(HIREDIS_LIB hiredis)
target_link_libraries(ortoa-host ${HIREDIS_LIB})

# <------------ add redis-plus-plus dependency -------------->
# NOTE: this should be *sw* NOT *redis++*
find_path(REDIS_PLUS_PLUS_HEADER sw)
target_include_directories(ortoa-host PUBLIC ${REDIS_PLUS_PLUS_HEADER})

find_library(REDIS_PLUS_PLUS_LIB redis++)
target_link_libraries(ortoa-host ${REDIS_PLUS_PLUS_LIB})
