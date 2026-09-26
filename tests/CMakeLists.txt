find_package(GTest QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.18.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Discover all test files
file(GLOB_RECURSE TEST_SOURCES "*.cc")

add_executable(alm_tests ${TEST_SOURCES})

target_link_libraries(alm_tests
    PRIVATE
        alm_domain
        GTest::gtest_main
)

target_include_directories(alm_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

include(GoogleTest)
gtest_discover_tests(alm_tests)
