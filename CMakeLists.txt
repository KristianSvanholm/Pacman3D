cmake_minimum_required(VERSION 3.15)

project(PacMan3D)

find_package(OpenGL REQUIRED)

add_subdirectory(glad)
add_subdirectory(glfw)
add_subdirectory(glm)

add_executable(PacMan3D  "main.cpp")
target_link_libraries(PacMan3D glfw glad OpenGL::GL ${CMAKE_DL_LIBS})
