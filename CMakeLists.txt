cmake_minimum_required(VERSION 3.20)
project(project C CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 3.3 REQUIRED)

add_executable(project
  src/main.cpp
  src/glad.c
)

target_include_directories(project PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(project PRIVATE
  glfw
  "-framework OpenGL"
  "-framework Cocoa"
  "-framework IOKit"
  "-framework CoreVideo"

)

if(APPLE)
    set(LIBOMP_DIR /opt/homebrew/opt/libomp)
    find_library(LIBOMP libomp.dylib HINTS ${LIBOMP_DIR}/lib)
    if(LIBOMP)
        message(STATUS "Found libomp: ${LIBOMP}")
        target_compile_options(project PUBLIC -Xpreprocessor -fopenmp)
        target_link_libraries(project PUBLIC ${LIBOMP})
        target_include_directories(project PUBLIC ${LIBOMP_DIR}/include)
    else()
        message(FATAL_ERROR "libomp not found. Install with: brew install libomp")
    endif()
endif()
