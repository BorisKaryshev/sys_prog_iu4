cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(project_name system_programming_hw5)

if(NOT DEFINED BUILD_MODE)
    set(BUILD_MODE "DEBUG")
endif()

project(${project_name})

set(source
    ${PROJECT_SOURCE_DIR}/src/main_module.c
    ${PROJECT_SOURCE_DIR}/src/string_utils.c
    ${PROJECT_SOURCE_DIR}/src/stack.c
    ${PROJECT_SOURCE_DIR}/src/pair.c
    ${PROJECT_SOURCE_DIR}/src/serialize.c
    ${PROJECT_SOURCE_DIR}/src/list.c
)
set(test_source
    ${PROJECT_SOURCE_DIR}/test/src/stack.cpp
    ${PROJECT_SOURCE_DIR}/test/src/list.cpp
    ${PROJECT_SOURCE_DIR}/test/src/pair.cpp
)

set(
    include_locations
    ${PROJECT_SOURCE_DIR}/include
)

set(includes
    ${PROJECT_SOURCE_DIR}/include/main.h
    ${PROJECT_SOURCE_DIR}/include/stack.h
)

add_custom_target(
    format
    COMMAND clang-format
    -i
    --style=file:${PROJECT_SOURCE_DIR}/configs/.clang-format
    ${includes} ${source} ${PROJECT_SOURCE_DIR}/src/main.cpp
)

find_package(Boost REQUIRED)
find_package(
    Boost
    REQUIRED COMPONENTS
    program_options
    log
    log_setup
    thread
)
set(libs Boost::program_options Boost::log Boost::log_setup Boost::thread)

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/googletest)

add_library(${project_name}_lib STATIC ${source})
target_include_directories(${project_name}_lib PUBLIC ${include_locations})
if(DEFINED libs)
    target_link_libraries(${project_name}_lib ${libs})
endif()

add_executable(${project_name} ${PROJECT_SOURCE_DIR}/src/main.cpp)
add_dependencies(${project_name} format)
target_link_libraries(${project_name} ${project_name}_lib)

if(ENABLE_MEMORY_SANITIZERS)
    add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
endif()

add_custom_target(
    exe
    DEPENDS ${project_name}
    COMMAND
    cp
    ${CMAKE_BINARY_DIR}/${project_name}
    ${CMAKE_BINARY_DIR}/exe
)

add_executable(test ${test_source})
target_link_libraries(test gtest_main)
target_link_libraries(test ${project_name}_lib)
target_link_libraries(test ${libs})
target_compile_options(test PRIVATE  -g3 -O0)

target_compile_options(${project_name} PRIVATE -Wall -Wextra -Werror -Wpedantic -pthread)
target_compile_options(${project_name}_lib PRIVATE  -Wall -Wextra -Werror -Wpedantic -pthread)

if(${BUILD_MODE} STREQUAL "DEBUG")
    target_compile_options(${project_name} PRIVATE  -g3 -O0)
    target_compile_options(${project_name}_lib PRIVATE  -g3 -O0)
elseif(${BUILD_MODE} STREQUAL "RELEASE")
    target_compile_options(${project_name} PRIVATE  -O2)
    target_compile_options(${project_name}_lib PRIVATE  -O2)
else()
    message(FATAL_ERROR "Unknow build mode")
endif()
