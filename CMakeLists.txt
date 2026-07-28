cmake_minimum_required(VERSION 3.12...3.26)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Release default")
endif()

project(tinyrenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(iwyu "Run include-what-you-use")
if(iwyu)
  find_program(IWYU_EXE NAMES include-what-you-use REQUIRED)
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_EXE})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|Intel")
  add_compile_options(-Wall)
endif()

find_package(OpenMP COMPONENTS CXX)

set(SOURCES main.cpp our_gl.cpp model.cpp tgaimage.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE $<$<BOOL:${OpenMP_CXX_FOUND}>:OpenMP::OpenMP_CXX>)

add_executable(print study/print.cpp model.cpp tgaimage.cpp)
target_include_directories(print PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(rasterize study/rasterize.cpp model.cpp tgaimage.cpp)
target_include_directories(rasterize PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(rasterize2 study/rasterize2.cpp model.cpp tgaimage.cpp)
target_include_directories(rasterize2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cut study/cut.cpp  tgaimage.cpp)
target_include_directories(cut PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

file(GENERATE OUTPUT .gitignore CONTENT "*")
