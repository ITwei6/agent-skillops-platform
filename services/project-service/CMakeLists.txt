add_executable(project-service
    source/main.cc
    source/service/project_service.cc
)

target_include_directories(project-service
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(project-service
    PRIVATE
        skillops::common
)
