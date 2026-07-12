add_executable(artifact-service
    source/main.cc
    source/service/artifact_service.cc
)

target_include_directories(artifact-service
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(artifact-service
    PRIVATE
        skillops::common
)
