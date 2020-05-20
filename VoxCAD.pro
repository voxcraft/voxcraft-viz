TEMPLATE = app
TARGET = VoxCAD
QT += core gui xml opengl concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += debug
DEFINES += QT_XML_LIB QT_OPENGL_LIB USE_OPEN_GL QT_DLL PREC_MED
INCLUDEPATH += \
    ./src/ui \
    ./src/VX1 \
    ./src/VX2/include \
    . \
    ./GeneratedFiles/$(Configuration) \
    ./GeneratedFiles \
    /usr/include/qwt
LIBS += \
#-lqwt \
#    -lOpenGL32 \
    -lGL \
 #   -lglu32 \
    -lglut \
    -lGLU \
    -lz \
    -lm
 #   -l qwt-qt4
DEPENDPATH += .
#DESTDIR = release
#MOC_DIR += ./GeneratedFiles/release
#OBJECTS_DIR += release
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
include(VoxCAD.pri)
