cmake_minimum_required(VERSION 3.10.0)
project(test VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

add_executable(test
    src/main.cpp
    src/camera_control.cpp
    src/gui_control.cpp
    #src/glad/src/glad.c  из за него всё по пизде пошло
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/backends/imgui_impl_glut.cpp
    src/imgui/backends/imgui_impl_opengl3.cpp 
)

target_include_directories(test PRIVATE
    ${GLUT_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
#   src/glad >:(
    src/imgui
    src/imgui/backends
)

target_link_libraries(test PRIVATE
    OpenGL::GL
    OpenGL::GLU
    GLUT::GLUT
    ${GLEW_LIBRARIES}
)
