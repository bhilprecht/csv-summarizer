cmake_minimum_required(VERSION 3.0.0)
project(csvsum VERSION 0.1.0)

include_directories(src/core)

find_package(Boost REQUIRED system)
include_directories(${BOOST_INCLUDE_DIRS})
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost Not found")
endif()

set(FORT_ENABLE_TESTING OFF CACHE INTERNAL "")
add_subdirectory(third-party/libfort)

add_executable(example src/examples/main.cpp)
target_link_libraries(example ${Boost_LIBRARIES} fort)

set(DOCTEST_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/doctest)
file(DOWNLOAD
    https://raw.githubusercontent.com/onqtam/doctest/2.4.6/doctest/doctest.h
    ${DOCTEST_DOWNLOAD_DIR}/doctest.h
    EXPECTED_HASH SHA1=40728d2bed7f074e80cb095844820114e7d16ce0
)

add_executable(test_csv_sum test/unittest_main.cpp)
target_include_directories(test_csv_sum PRIVATE ${DOCTEST_DOWNLOAD_DIR})
target_link_libraries(test_csv_sum ${Boost_LIBRARIES} fort)

enable_testing()
add_test(test_csv_sum test_csv_sum)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
