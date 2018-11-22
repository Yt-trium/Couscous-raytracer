cmake_minimum_required(VERSION 2.8)

# ---------------------------------------------
# Project settings and dependencies.
# ---------------------------------------------

project(Couscous-raytracer)

find_package (Qt4 REQUIRED)
find_package (OpenGL REQUIRED)
find_package(assimp REQUIRED)

set (QT_USE_QTOPENGL TRUE)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# ---------------------------------------------
# Project files.
# ---------------------------------------------

set (sources
    src/main.cpp
)

list (APPEND couscous_sources
    ${sources})

set (renderer_sources
    src/renderer/aabb.cpp
    src/renderer/aabb.h
    src/renderer/camera.cpp
    src/renderer/camera.h
    src/renderer/gridaccelerator.cpp
    src/renderer/gridaccelerator.h
    src/renderer/material.cpp
    src/renderer/material.h
    src/renderer/photonMapping.cpp
    src/renderer/photonMapping.h
    src/renderer/ray.cpp
    src/renderer/ray.h
    src/renderer/render.cpp
    src/renderer/render.h
    src/renderer/rng.cpp
    src/renderer/rng.h
    src/renderer/samplegenerator.cpp
    src/renderer/samplegenerator.h
    src/renderer/visualobject.cpp
    src/renderer/visualobject.h
    src/renderer/utility.cpp
    src/renderer/utility.h
)

list (APPEND couscous_sources
    ${renderer_sources})

set (gui_sources
    src/gui/dialogmaterial.cpp
    src/gui/dialogmaterial.h
    src/gui/dialogmaterial.ui
    src/gui/dialogobject.cpp
    src/gui/dialogobject.h
    src/gui/dialogobject.ui
    src/gui/frameviewer.cpp
    src/gui/frameviewer.h
    src/gui/mainwindow.cpp
    src/gui/mainwindow.h
    src/gui/mainwindow.ui
    src/gui/scene.cpp
    src/gui/scene.h
)

list (APPEND couscous_sources
    ${gui_sources})

set (io_sources
    src/io/scenefilereader.cpp
    src/io/scenefilereader.h
)

list (APPEND couscous_sources
    ${io_sources})

set (common_sources
    src/common/logger.cpp
    src/common/logger.h
)

list (APPEND couscous_sources
    ${common_sources})

set (test_sources
    src/test/catch.hpp
    src/test/test.cpp
    src/test/test.h
)

list (APPEND couscous_sources
    ${test_sources})

set (resources
    icons/icons.qrc
)

# ---------------------------------------------
# Compiler options.
# ---------------------------------------------

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    if (CMAKE_BUILD_TYPE EQUAL "RELEASE")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3")
    endif()
endif()

# ---------------------------------------------
# Compie Qt files.
# ---------------------------------------------

QT4_ADD_RESOURCES (couscous_resource_files
    ${resources}
)

# ---------------------------------------------
# Create executable.
# ---------------------------------------------

include (${QT_USE_FILE})
include_directories(
    src
    ${ASSIMP_INCLUDE_DIR}
    distant)

add_executable(${PROJECT_NAME} ${couscous_sources} ${couscous_resource_files})

target_link_libraries (${PROJECT_NAME}
    Qt4::QtGui
    Qt4::QtCore
    Qt4::QtOpenGL
    Qt4::QtMultimedia
    ${OPENGL_LIBRARIES}
    ${ASSIMP_LIBRARIES}
)

