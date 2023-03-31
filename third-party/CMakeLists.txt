cmake_minimum_required(VERSION 3.12)

# == GLFW ==
add_subdirectory(GLFW)

# == glad ==
add_library(glad glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

# == glm ==
add_subdirectory(glm)

# == stb == 
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb/)

# == garamon (C3GA) == 
add_subdirectory(garamon_c3ga INTERFACE)
target_include_directories(c3ga INTERFACE garamon_c3ga/src)
