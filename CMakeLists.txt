cmake_minimum_required(VERSION 3.0.0)
project(csvsum VERSION 0.1.0)

include(CTest)
enable_testing()

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

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
