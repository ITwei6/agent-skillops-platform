file(GLOB project_odb_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/skillops/project/project_record.hxx
)

execute_process(
    COMMAND odb -d mysql --std c++17 --generate-query --generate-schema --profile boost/date-time --output-dir ${CMAKE_CURRENT_BINARY_DIR} ${project_odb_headers}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB project_odb_sources
    ${CMAKE_CURRENT_BINARY_DIR}/*-odb.cxx
)

add_executable(project-service
    source/main.cc
    source/service/project_service.cc
    ${project_odb_sources}
)

target_include_directories(project-service
    PRIVATE
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/skillops/project
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(project-service
    PRIVATE
        skillops::common
)
