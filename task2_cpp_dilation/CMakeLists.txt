cmake_minimum_required(VERSION 3.16)
project(morphology_dilation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# 头文件接口库：算法本体（仅头文件，便于复用）
add_library(morphology INTERFACE)
target_include_directories(morphology INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 演示程序
add_executable(morph_demo src/main.cpp)
target_link_libraries(morph_demo PRIVATE morphology)
target_compile_options(morph_demo PRIVATE -Wall -Wextra)

# 单元测试
add_executable(morph_tests tests/test_morphology.cpp)
target_link_libraries(morph_tests PRIVATE morphology)
target_compile_options(morph_tests PRIVATE -Wall -Wextra)

enable_testing()
add_test(NAME morphology_unit_tests COMMAND morph_tests)
