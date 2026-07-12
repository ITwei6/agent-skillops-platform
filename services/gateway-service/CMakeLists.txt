add_executable(gateway-service
    source/main.cc
    source/server/simple_http_server.cc
)

target_include_directories(gateway-service
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(gateway-service
    PRIVATE
        skillops::common
)
