add_executable(experience-service
    source/main.cc
    source/service/experience_service.cc
)

target_include_directories(experience-service
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(experience-service
    PRIVATE
        skillops::common
)
