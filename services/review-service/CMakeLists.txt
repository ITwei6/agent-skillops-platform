add_executable(review-service
    source/main.cc
    source/service/review_service.cc
)

target_include_directories(review-service
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/services/common/include
)

target_link_libraries(review-service
    PRIVATE
        skillops::common
)
