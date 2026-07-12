add_executable(skill-service
    source/main.cc
    source/service/skill_service.cc
)

target_include_directories(skill-service
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(skill-service
    PRIVATE
        skillops::common
)
