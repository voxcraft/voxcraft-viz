cmake_minimum_required(VERSION 3.5)
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(BUILD_STATIC_LIBS "Build static library" ON)

project(voxcraft-viz LANGUAGES CXX)
include_directories(/usr/include)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VOXCAD_MAIN src/main)
set(VX1 src/VX1)
set(VX2 src/VX2)
set(QTUTILS src/QTUtils)

file(GLOB VOXCAD_MAIN_SRC ${VOXCAD_MAIN}/*.c* ${VOXCAD_MAIN}/*.h)
file(GLOB VX2_SRC ${VX2}/src/*.c* ${VX2}/src/*.h)
file(GLOB VX1_SRC ${VX1}/*.c* ${VX1}/*.h)
file(GLOB QTUTILS_SRC ${QTUTILS}/*.c* ${QTUTILS}/*.h)
file(GLOB QCUSTOMPLOT_SRC src/qcustomplot/*.c* src/qcustomplot/*.h)
add_executable(voxcraft-viz VoxCad.qrc ${VOXCAD_MAIN_SRC} ${VX2_SRC} ${VX1_SRC} ${QTUTILS_SRC} ${QCUSTOMPLOT_SRC})
install(TARGETS voxcraft-viz DESTINATION bin)

# add_definitions(-DQT_XML_LIB -DQT_OPENGL_LIB -DUSE_ZLIB_COMPRESSION -DUSE_OPEN_GL -DQT_DLL -DPREC_MED -DUSE_OMP)
add_definitions(-DQT_XML_LIB -DQT_OPENGL_LIB -DUSE_OPEN_GL -DQT_DLL -DPREC_MED -DUSE_OMP)

# Add module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

target_include_directories(voxcraft-viz PUBLIC ${VOXCAD_MAIN} ${VX2}/include ${VX1} ${QTUTILS} src/qcustomplot)
find_package(Qt5 COMPONENTS Core Gui Xml OpenGL Concurrent Widgets PrintSupport REQUIRED)
target_link_libraries(voxcraft-viz PRIVATE Qt5::Core Qt5::Gui Qt5::Xml Qt5::OpenGL Qt5::Concurrent Qt5::Widgets Qt5::PrintSupport)

#for Boost
find_package(Boost REQUIRED)
if (Boost_FOUND)
    message(STATUS "Boost FOUND")
    target_include_directories(voxcraft-viz PRIVATE ${Boost_INCLUDE_DIRS})
endif()

#for visualization
set(OpenGL_GL_PREFERENCE GLVND)
if (UNIX)
    find_package(OpenGL REQUIRED) # find and include OpenGL
    if (OpenGL_FOUND)
        message(STATUS "OPENGL FOUND")
        target_include_directories(voxcraft-viz PRIVATE ${OpenGL_INCLUDE_DIRS})
        if (APPLE)
            target_link_libraries(voxcraft-viz PRIVATE ${OpenGL_LIBRARIES})
        else()
            target_link_libraries(voxcraft-viz PRIVATE ${OpenGL_LIBRARIES} GL)
        endif()
        target_link_libraries(voxcraft-viz PRIVATE ${OPENGL_glu_LIBRARY})
    endif()
else()
    find_package(OPENGL REQUIRED) # find and include OpenGL
    if (OPENGL_FOUND)
        message(STATUS "OPENGL FOUND")
        target_include_directories(voxcraft-viz PRIVATE ${OPENGL_INCLUDE_DIRS})
        target_link_libraries(voxcraft-viz PRIVATE ${OPENGL_LIBRARIES})
    endif()
endif()

find_package(glfw3 CONFIG REQUIRED)
if (glfw3_FOUND)
    message(STATUS "GLFW FOUND")
    target_include_directories(voxcraft-viz PRIVATE ${glfw3_INCLUDE_DIRS})
    target_link_libraries(voxcraft-viz PRIVATE ${glfw3_LIBRARIES})
endif()

find_package(GLUT REQUIRED) # GLEW
if (GLUT_FOUND)
    message(STATUS "GLUT FOUND")
    target_include_directories(voxcraft-viz PRIVATE ${GLUT_INCLUDE_DIRS})
    if (APPLE)
        target_link_libraries(voxcraft-viz PRIVATE ${GLUT_LIBRARIES})
    else()
        target_link_libraries(voxcraft-viz PRIVATE ${GLUT_LIBRARIES} glut)
    endif()
endif()

if (APPLE)
    message("APPLE doesn't need glm.")
else()
    find_package(glm CONFIG REQUIRED) # glm
    if (glm_FOUND)
        message(STATUS "GLM FOUND")
        target_include_directories(voxcraft-viz PRIVATE ${glm_INCLUDE_DIRS})
        target_link_libraries(voxcraft-viz PRIVATE ${glm_LIBRARIES} glm)
    endif()
endif()

