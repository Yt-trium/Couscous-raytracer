#-------------------------------------------------
#
# Project created by QtCreator 2018-09-09T14:43:03
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = Couscous-raytracer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        src/main.cpp \
        src/gui/mainwindow.cpp \
    src/renderer/ray.cpp \
    src/renderer/render.cpp \
    src/renderer/rng.cpp \
    src/renderer/visualobject.cpp \
    src/renderer/camera.cpp \
    src/gui/frameviewer.cpp \
    src/renderer/material.cpp \
    src/renderer/samplegenerator.cpp \
    src/test/test.cpp \
    src/renderer/utility.cpp \
    src/renderer/photonMapping.cpp \
    src/renderer/gridaccelerator.cpp \
    src/renderer/aabb.cpp \
    src/gui/scene.cpp \
    src/gui/dialogmaterial.cpp \
    src/gui/dialogmeshfile.cpp \
    src/gui/dialogobject.cpp \
    src/io/filereader.cpp \
    src/common/logger.cpp

HEADERS += \
        src/gui/mainwindow.h \
    src/renderer/ray.h \
    src/renderer/render.h \
    src/renderer/rng.h \
    src/renderer/visualobject.h \
    src/renderer/camera.h \
    src/gui/frameviewer.h \
    src/renderer/material.h \
    src/renderer/samplegenerator.h \
    src/test/catch.hpp \
    src/test/test.h \
    src/renderer/utility.h \
    src/renderer/photonMapping.h \
    src/renderer/aabb.h \
    src/renderer/gridaccelerator.h \
    src/gui/scene.h \
    src/gui/dialogmaterial.h \
    src/gui/dialogmeshfile.h \
    src/gui/dialogobject.h \
    src/io/filereader.h \
    src/common/logger.h

FORMS += \
        src/gui/mainwindow.ui \
    src/gui/dialogmaterial.ui \
    src/gui/dialogmeshfile.ui \
    src/gui/dialogobject.ui

INCLUDEPATH += glm src distant

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons/icons.qrc
