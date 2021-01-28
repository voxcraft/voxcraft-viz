# Voxcraft

[![voxcraft-viz](https://snapcraft.io//voxcraft-viz/badge.svg)](https://snapcraft.io/voxcraft-viz)
 
[Voxcraft-viz](https://github.com/voxcraft/voxcraft-viz), which was originally forked from [VoxCAD](https://github.com/jonhiller/VoxCAD), is a visualization tool for [voxcraft-sim](https://github.com/voxcraft/voxcraft-sim).

voxcraft-viz can be used to [replay the behavior of a robot](https://www.youtube.com/embed/ytQ7dj3yE8M) that has already been simulated using voxcraft-sim.

There are several demos in the `demos` folder. To view one of those demos, use this command: 

```bash
voxcraft-viz demos/m190.history
```

You can also [draw a robot in voxcraft-viz](https://www.youtube.com/embed/jvxQjlrLgQo) using your mouse and then press play to watch its behavior.


You can learn more about the whole voxcraft project (not just software) [here](https://voxcraft.github.io/).



# Installation

## On Linux
```
sudo snap install voxcraft-viz
```

The first time executing `voxcraft-viz` will take 2 minutes, due to that snap need to set up a sandbox for the executable.

More detail about installation: 

[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-white.svg)](https://snapcraft.io/voxcraft-viz)

## On Mac

We don't provide binary version for Mac (because the developers cannot afford Mac's). However, you can build the project from source code. [Here is a simple tutorial for that](InstallOnMac.md).


## On Windows
On Win10, you can download the software 
[here](https://github.com/voxcraft/voxcraft-viz/raw/master/bin/voxcraft-viz-win10-x64.7z).



