# Use the edger8r to generate C bindings from the EDL file.
add_custom_command(
  OUTPUT ortoa_t.h ortoa_t.c ortoa_args.h
  DEPENDS ${CMAKE_SOURCE_DIR}/src/ortoa.edl
  COMMAND
    openenclave::oeedger8r ${CMAKE_SOURCE_DIR}/src/ortoa.edl --trusted
    --search-path ${OE_INCLUDEDIR} 
    --search-path ${OE_INCLUDEDIR}/openenclave/edl/sgx
)

set(CRYPTO_SRC ${OE_CRYPTO_LIB}_src)
if (OE_CRYPTO_LIB STREQUAL "openssl_3")
  set(CRYPTO_SRC openssl_src)
endif ()

add_executable(ortoa-enc 
    common/ecalls.cpp
    ../crypto/encryption_engine.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/ortoa_t.c
)

target_compile_definitions(ortoa-enc PUBLIC OE_API_VERSION=2)

target_include_directories(
  ortoa-enc
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} # Needed for #include "../shared.h"
          ${CMAKE_CURRENT_BINARY_DIR} 
          ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ortoa-enc 
    openenclave::oeenclave 
    openenclave::oecrypto${OE_CRYPTO_LIB}
    openenclave::oelibcxx
)