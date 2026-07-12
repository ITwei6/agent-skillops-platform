file(GLOB identity_odb_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/skillops/identity/user_record.hxx
)

execute_process(
    COMMAND odb -d mysql --std c++17 --generate-query --generate-schema --profile boost/date-time --output-dir ${CMAKE_CURRENT_BINARY_DIR} ${identity_odb_headers}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB identity_odb_sources
    ${CMAKE_CURRENT_BINARY_DIR}/*-odb.cxx
)

add_executable(identity-service
    source/main.cc
    source/service/identity_service.cc
    ${identity_odb_sources}
)

target_include_directories(identity-service
    PRIVATE
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/skillops/identity
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(identity-service
    PRIVATE
        skillops::common
)
