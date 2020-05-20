# Voxcraft

[![voxcraft-viz](https://snapcraft.io//voxcraft-viz/badge.svg)](https://snapcraft.io/voxcraft-viz)

This repo is one of the three parts of voxcraft software.

1. [voxcraft-sim](https://github.com/voxcraft/voxcraft-sim): A highly parallelized physics engine that can simulate the voxel-based soft robots. This part utilizes CUDA and GPU.

2. voxcraft-evo: The evolutionary algorithms that can automatically design voxel-based soft robots.

3. [voxcraft-viz](https://github.com/voxcraft/voxcraft-viz): The visualization tool that can playback the history file produced by voxcraft-sim, and it can used for manually design bots and run it in a CPU-based physics engine.

Learn more about the whole voxcraft project (not just software) to get a bigger picture, please refer to: https://voxcraft.github.io/

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

# Demos

There are several demos in the `demos` folder. To view one of those demos, use this command: 

```bash
voxcraft-viz demos/m190.history
```

# Screenshot

![screenshot](https://res.cloudinary.com/canonical/image/fetch/f_auto,q_auto,fl_sanitize,w_819,h_443/https://dashboard.snapcraft.io/site_media/appmedia/2020/05/2_lgb5miW.png)

