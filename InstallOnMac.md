# Install voxcraft-viz On Mac

```bash
brew install cmake
brew install boost
brew install qt5
brew install glfw3
brew cask install xquartz
brew install freeglut
brew install glm
brew install mesa
```

this will take a while...

```bash
git clone https://github.com/liusida/voxcraft-viz.git

cd voxcraft-viz/
mkdir build
cd build/

cmake -DQt5_DIR=$(brew --prefix qt5)/lib/cmake/Qt5 -DCMAKE_BUILD_TYPE=Release ..

make -j 10
./voxcraft-viz
```

Done!

An executable `voxcraft-viz` was produced, and it is all you need. You can copy it somewhere in your $PATH, such as `/usr/bin`.


Thank Arlo Cohen at University of Vermont for providing the instruction on Mac.

Thank Caitlin Grasso for helping us simplify this instruction during installation. 
