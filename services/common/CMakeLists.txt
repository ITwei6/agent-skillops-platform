add_library(skillops-common
    source/http/http_client.cc
    source/http/http_server.cc
)

target_include_directories(skillops-common
    PUBLIC
        include
)

target_link_libraries(skillops-common
    PUBLIC
        skillops::microservice_kit
        Threads::Threads
)

add_library(skillops::common ALIAS skillops-common)
