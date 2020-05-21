TEMPLATE = app
TARGET = voxcraft-viz
QT += core gui xml opengl concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
CONFIG += release
DEFINES += QT_XML_LIB QT_OPENGL_LIB USE_OPEN_GL QT_DLL PREC_MED
INCLUDEPATH += \
    . \
    ./src/ui \
    ./src/VX1 \
    ./src/VX2/include \
    ./src/QUtils \
    ./src/qcustomplot \
    ./GeneratedFiles/$(Configuration) \
    ./GeneratedFiles
#    /usr/include/qwt
LIBS += \
# Windows
    -lOpenGL32 \
    -lglu32 \
# Linux
#    -lGL \
#    -lglut \
#    -lGLU \

    -lz \
    -lm
################
#   -lqwt-qt4 \
#   -lqwt \
DEPENDPATH += .
#DESTDIR = release
#MOC_DIR += ./GeneratedFiles/release
#OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
include(voxcraft-viz.pri)