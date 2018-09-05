cmake_minimum_required(VERSION 2.8)

set (sources
    src/main.cpp
)

project(Couscous-raytracer)

find_package (Qt4 REQUIRED)
set (QT_USE_QTOPENGL TRUE)
include (${QT_USE_FILE})

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries (${PROJECT_NAME}
    ${OPENGL_LIBRARY}
)
