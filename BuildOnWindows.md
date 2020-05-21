# Build On Windows

## Install git

Download and install from https://git-scm.com/download/win

Git clone this repo.

## Install QT5 open source

Download and install QT5 for open source from https://www.qt.io/download-open-source

The download link is at the bottom of the page

During installation, select Kit: QT x.xx -> MinGW x.xx, select Tools: MinGW x.xx, make sure two MinGW are the same version.

## Install MESA3D FOR WINDOWS

Download from https://fdossena.com/?p=mesa/index.frag

It is a DLL file (opengl32.dll). Later on, we need to put the dll in the folder where the executable will be, so that the executable will use this opengl32.dll instead of Microsoft Windows' default opengl32.dll.

## Open QT Creator


