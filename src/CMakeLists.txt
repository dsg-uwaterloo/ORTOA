include(FetchContent)
include(GNUInstallDirs)

# Setting the cmake compiler when LVI mitigation is not enabled. If the CC
# environment variable has been specified or the if CMAKE_C_COMPILER cmake
# variable has been passed to cmake, use the C compiler that has been specified.
# Otherwise, prefer clang. Same for C++ compiler. This must be done before
# the `project` command.
if (UNIX)
  if (NOT DEFINED ENV{CC} AND NOT DEFINED CMAKE_C_COMPILER)
    find_program(CMAKE_C_COMPILER clang-11 clang-10 clang)
  endif ()
  if (NOT DEFINED ENV{CXX} AND NOT DEFINED CMAKE_CXX_COMPILER)
    find_program(CMAKE_CXX_COMPILER clang++-11 clang++-10 clang++)
  endif ()
endif ()

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()

# External Dependencies (FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0  # Specify the version you want
)

FetchContent_MakeAvailable(spdlog)


# Currently the `OpenEnclave` package depends on `project()`.
find_package(OpenEnclave CONFIG REQUIRED)


set(CMAKE_CXX_STANDARD 17)

# Set the OE_CRYPTO_LIB variable
set(OE_CRYPTO_LIB 
    openssl
    CACHE STRING "Crypto library used by enclaves."
)



# Generate key
add_custom_command(
  OUTPUT private.pem public.pem
  COMMAND openssl genrsa -out private.pem -3 3072
  COMMAND openssl rsa -in private.pem -pubout -out public.pem
)

# Sign enclave
add_custom_command(
  OUTPUT enclave/ortoa-enc.signed
  DEPENDS enclave enclave/ortoa.conf private.pem
  COMMAND openenclave::oesign sign -e $<TARGET_FILE:ortoa-enc> -c
          ${CMAKE_SOURCE_DIR}/src/enclave/ortoa.conf -k private.pem
) 

add_custom_target(sign ALL DEPENDS enclave/ortoa-enc.signed)


# Thrift
add_custom_command(
  OUTPUT 
    gen-cpp/RPC.h
    gen-cpp/RPC.cpp
    gen-cpp/Operation_types.h
    gen-cpp/Operation_types.cpp
  COMMAND thrift -r --gen cpp ${CMAKE_SOURCE_DIR}/src/thrift/Operation.thrift
)

set_source_files_properties(
  ${CMAKE_BINARY_DIR}/gen-cpp/RPC.h
  ${CMAKE_BINARY_DIR}/gen-cpp/RPC.cpp  
  ${CMAKE_BINARY_DIR}/gen-cpp/Operation_types.h 
  ${CMAKE_BINARY_DIR}/gen-cpp/Operation_types.cpp 
  
  PROPERTIES GENERATED TRUE
)

add_custom_target(generate-thrift
  DEPENDS
    gen-cpp/Operation_types.cpp
    gen-cpp/Operation_types.h
    gen-cpp/RPC.cpp
    gen-cpp/RPC.h
)


# Client
add_subdirectory(client)

# Host
add_subdirectory(host)

# Enclave
add_subdirectory(enclave)

# Storage library
add_subdirectory(libstorage)
