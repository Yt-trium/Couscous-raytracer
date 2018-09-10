cmake_minimum_required(VERSION 2.8)

set (sources
    src/main.cpp
    src/ray.h
    src/ray.cpp
    src/render.cpp
    src/render.h
    src/usualshapes.cpp
    src/usualshapes.h
    src/visualobject.h
    src/visualobjectlist.cpp
    src/visualobjectlist.h
)

list (APPEND couscous_sources
    ${sources})

set (renderer_sources
    src/renderer/camera.cpp
    src/renderer/camera.h
)

list (APPEND couscous_sources
    ${renderer_sources})

set (gui_srouces
    src/gui/mainwindow.cpp
    src/gui/mainwindow.h
    src/gui/mainwindow.ui
)

list (APPEND couscous_sources
    ${gui_sources})

project(Couscous-raytracer)

find_package (Qt4 REQUIRED)
set (QT_USE_QTOPENGL TRUE)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include (${QT_USE_FILE})
include_directories(src)

add_executable(${PROJECT_NAME} ${couscous_sources})

target_link_libraries (${PROJECT_NAME}
    Qt4::QtGui
    ${OPENGL_LIBRARY}
)
