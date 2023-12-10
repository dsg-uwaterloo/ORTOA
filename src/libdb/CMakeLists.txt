set(LIBRARY_NAME ortoadb)

add_library(${LIBRARY_NAME} SHARED "")
add_subdirectory(src)

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# <------------ add hiredis dependency --------------->
find_path(HIREDIS_HEADER hiredis)
target_include_directories(${LIBRARY_NAME} PUBLIC ${HIREDIS_HEADER})

find_library(HIREDIS_LIB hiredis)
target_link_libraries(${LIBRARY_NAME} ${HIREDIS_LIB})

# <------------ add redis-plus-plus dependency -------------->
# NOTE: this should be *sw* NOT *redis++*
find_path(REDIS_PLUS_PLUS_HEADER sw)
target_include_directories(${LIBRARY_NAME} PUBLIC ${REDIS_PLUS_PLUS_HEADER})

find_library(REDIS_PLUS_PLUS_LIB redis++)
target_link_libraries(${LIBRARY_NAME} ${REDIS_PLUS_PLUS_LIB})

install(TARGETS ${LIBRARY_NAME}
    LIBRARY DESTINATION lib
)

# target_link_libraries(${LIBRARY_NAME} 
#     hiredis
#     redis++
#     /usr/local/lib/libredis++.a
#     /usr/local/lib/libhiredis.a
# )
