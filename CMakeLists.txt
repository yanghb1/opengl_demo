cmake_minimum_required(VERSION 3.5)

project(opengl_demo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt5 包
find_package(Qt5 COMPONENTS Core Widgets OpenGL REQUIRED)
# 查找 OpenGL 包
find_package(OpenGL REQUIRED)

# link_directories(${CMAKE_SOURCE_DIR}/lib/)

add_executable(opengl_demo
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    glwidget.h
    glwidget.cpp
)

target_link_libraries(opengl_demo PRIVATE Qt5::Core Qt5::Widgets Qt5::OpenGL ${OPENGL_LIBRARIES})


