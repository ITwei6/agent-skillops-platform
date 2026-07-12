cmake_minimum_required(VERSION 3.16)

project(agent_skillops_platform VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(SkillOpsDependencies)
find_package(Threads REQUIRED)

add_subdirectory(services/common)
add_subdirectory(services/gateway-service)
add_subdirectory(services/identity-service)
add_subdirectory(services/project-service)
