add_executable(identity-service
    source/main.cc
)

target_include_directories(identity-service
    PRIVATE
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(identity-service
    PRIVATE
        skillops::microservice_kit
)
