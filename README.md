# LibDeck

Creates an SDL2.0 Representation of a Deck with a settings menu and events relay.


The goal of this project is to display HID inputs for project [Blurrg](https://github.com/bluemner/blurrg). It was broken out into its own repo as some users may want to use the screen / code for other projects.


> **Note:** This code is still in **alpha**, there will be bugs and other issues.
>  The Application Binary Interface (ABI) is not yet finalized. Expect changes before stable release.


## Setup

### Debian \ Ubuntu

```bash
sudo apt install build-essential # compiler is gcc 
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev # SDL2.0
```

### Arch \ SteamOS

```bash
sudo pacman -S base-devel sdl2 sdl2_image sdl2_ttf
```

> **Note:** Steam OS is read only by default
>```bash
> sudo steamos-readonly disable
> sudo pacman-key --init
> sudo pacman-key --populate archlinux
> ```


## Build

- `make all` will create **debug** and **release** builds. Debug builds have address sanitizer turned on
- `make help` will show make commands.


## Licenses and Dependencies

`Libdeck` is under the LGPL-2.1-only [see LICENSE](./LICENSE); 

[source](./source) / [include](./include) files should be marked with `SPDX-License-Identifier` with the correct license used. 


### Example
The code repo has an example that can help developer get started.
* [./example/demo.c](./example/demo.c) Licensed under BSD-3-Clause

### Assets
This project uses the following fonts from [material-design-icons](https://github.com/google/material-design-icons)
* **[./assets/MaterialSymbolsOutlined.ttf](./assets/MaterialSymbolsOutlined.ttf)** – Licensed under the [Apache License 2.0](https://apache.org)
* **[./assets/Roboto-Regular.ttf](./assets/Roboto-Regular.ttf)** – Licensed under the [Apache License 2.0](https://apache.org)

### Libraries

The project uses the following libraries:
* **SDL 2.0** – Dynamically linked and used under the [zlib License](https://wiki.libsdl.org/SDL2/FAQLicensing).
