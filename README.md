# PLACEHOLDER (Project Name)

## Overview
PLACEHOLDER is a 2D shooter game developed in C, showcasing traditional arcade gameplay with modern graphical elements. This game is designed to be accessible on multiple platforms, offering a robust, entertaining experience for gamers.

![Game Image](image.png)

Explore the gameplay and latest developments on 
[X](https://x.com/birajtwr).

## Prerequisites
To build and run PLACEHOLDER, ensure you have the following installed:

### General Dependencies:
- `make`
- `gcc`
- `git`

### Platform-Specific Dependencies:
#### Linux
- Debian:
  ```bash
  sudo apt-get install libsdl2-2.0-0 libsdl2-ttf-2.0-0 libsdl2-image-2.0-0 libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
  ```
- Fedora:
  ```bash
  sudo dnf install SDL2 SDL2-devel SDL2_image SDL2_image-devel SDL2_ttf SDL2_ttf-devel
  ```
- Other Distros:
  Check the repositories for the necessary libraries and headers, or compile from source as described on the [SDL2 Wiki](https://wiki.libsdl.org/SDL2/Installation).

#### Windows
- MinGW:
  - Download the latest releases of SDL2, SDL2_image, and SDL2_ttf from their respective GitHub repositories.
  - Extract the non-devel files to the project root, and copy the `.dll` files there.
  - Extract the `-devel-` files, navigate to their directories, and execute `make native` to install headers at `/usr`.

## Building the Game
To compile the game, navigate to the project root and run:

```bash
make
```
This generates an executable `game` on Linux or `game.exe` on Windows.

## Game Controls
- **A / LeftArrow**: Move left
- **D / RightArrow**: Move right
- **W / Space / UpArrow**: Jump or use special ability
- **K**: Toggle debug mode (shows a red overlay on entities)
- **F**: Fire weapon

# Contributors 🧑🏻‍💻
<a href="https://github.com/KenniBlank/ShooterGame2D/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=KenniBlank/ShooterGame2D" />
</a>





## Credits
- **Sprites**: All sprites used in this game are sourced from [Craftpix.net](https://craftpix.net)
- **Font**: Game typography provided by [Google Fonts](https://fonts.google.com)

## Documentation
For detailed developer documentation, refer to the [SDL2 Documentation](https://wiki.libsdl.org/FrontPage).

## Development Updates
Keep an eye on this section for the latest updates and progress notes. We aim to keep the community informed and engaged with continuous improvements and new features.
Explore the gameplay and latest developments on 
[X](https://x.com/birajtwr).
