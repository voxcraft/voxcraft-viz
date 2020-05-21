# Build On Windows

## Install git

Download and install from https://git-scm.com/download/win

Git clone this repo.

## Install QT5 open source

Download and install QT5 for open source from https://www.qt.io/download-open-source

The download link is at the bottom of the page

During installation, select Kit: QT x.xx -> MinGW x.xx, select Tools: MinGW x.xx, make sure two MinGW are the same version.

## Download Boost C++ Library

Refer to https://www.boost.org/doc/libs/1_73_0/more/getting_started/windows.html

Download from https://www.boost.org/users/history/version_1_73_0.html

Extract files to C:\boost\, so the boost path will be C:\boost\boost_1_73_0 (If you change this path, please change the path in voxcraft-viz.pro accordingly.)

## Install MESA3D FOR WINDOWS

Download from https://fdossena.com/?p=mesa/index.frag

It is a DLL file (opengl32.dll). Later on, we need to put the dll in the folder where the executable will be, so that the executable will use this opengl32.dll instead of Microsoft Windows' default opengl32.dll.

## Open QT Creator

Open the project file: voxcraft-viz.pro

Switch the project from Debug to Release Mode

Build

## Use MESA

Now if you run it, you will see the OpenGL area as a black screen.

Put downloaded opengl32.dll into the folder which contains the newly built executable.

Run it.

## Deliver the executable

Refer to https://wiki.qt.io/Deploy_an_Application_on_Windows

Make a new folder

Copy those files from `release/` folder
```
voxcraft-viz.exe
opengl32.dll
```

Copy those files from `c:\QT\QTx.x.x\bin\`
```
...dll
```

Copy this folder from `C:\QT\QTx.x.x\plugins\`
```
platforms/
```

In all, the deliverable folder will look like this:
```
.
├── libgcc_s_seh-1.dll
├── libstdc++-6.dll
├── libwinpthread-1.dll
├── opengl32.dll
├── platforms
│   ├── qdirect2d.dll
│   ├── qminimal.dll
│   ├── qoffscreen.dll
│   └── qwindows.dll
├── Qt5Core.dll
├── Qt5Gui.dll
├── Qt5OpenGL.dll
├── Qt5PrintSupport.dll
├── Qt5Widgets.dll
├── Qt5Xml.dll
└── voxcraft-viz.exe

1 directory, 15 files
```