set(LIBRARY_NAME libcommon)

add_library(${LIBRARY_NAME} SHARED "")
add_subdirectory(src)

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${LIBRARY_NAME}
    # openenclave::oeenclave 
    # openenclave::oecrypto${OE_CRYPTO_LIB}
    # openenclave::oelibcxx
    crypto
    ssl
)

install(TARGETS ${LIBRARY_NAME}
    LIBRARY DESTINATION lib
)
