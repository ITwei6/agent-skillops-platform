add_executable(project-service
    source/main.cc
)

target_include_directories(project-service
    PRIVATE
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(project-service
    PRIVATE
        skillops::microservice_kit
)
