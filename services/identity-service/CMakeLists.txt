add_executable(identity-service
    source/main.cc
    source/service/identity_service.cc
)

target_include_directories(identity-service
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(identity-service
    PRIVATE
        skillops::common
)
