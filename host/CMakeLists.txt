
add_custom_command(
  OUTPUT ortoa_u.h ortoa_u.c ortoa_args.h
  DEPENDS ${CMAKE_SOURCE_DIR}/ortoa.edl
  COMMAND
    openenclave::oeedger8r --untrusted ${CMAKE_SOURCE_DIR}/ortoa.edl
    --search-path ${OE_INCLUDEDIR} 
    --search-path ${OE_INCLUDEDIR}/openenclave/edl/sgx)

add_executable(ortoa-host
               host.cpp 
               redis.cpp
               redis.h
               ${CMAKE_CURRENT_BINARY_DIR}/ortoa_u.c)

target_include_directories(
  ortoa-host
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} # Needed for #include "../shared.h"
          ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(ortoa-host 
    openenclave::oehost 
    /usr/local/lib/libredis++.a
    /usr/local/lib/libhiredis.a
    rt
)
