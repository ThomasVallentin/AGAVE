cmake_policy(SET CMP0048 NEW)
project(GA_Projet 
    VERSION 0.0.1 
    LANGUAGES CXX C
)

cmake_minimum_required(VERSION 3.18)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG") # define DEBUG macro for debug builds

add_subdirectory(third-party)

find_package(ImGui 1.89 REQUIRED)
find_package(Eigen3 REQUIRED)

file(GLOB_RECURSE SOURCES_FILES "${PROJECT_SOURCE_DIR}/src/**.cpp")

add_executable(GA_Projet 
    ${SOURCES_FILES}
    ${ImGui_DEMO_SOURCES}
    ${ImGui_SOURCES})

target_include_directories(GA_Projet PUBLIC 
    src
    third-party/glad/include
    ${ImGui_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR})

target_link_libraries(GA_Projet 
    glad
    glfw
    glm
    stb
    c3ga)

target_compile_features(GA_Projet PRIVATE cxx_std_17)
