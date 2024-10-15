# ShooterGame2D
Creating a game in C like a chad;
![Game Image](image.png)
Check out [X](https://x.com/birajtwr) for gameplay of the latest development!

## Prerequisites
- General:
  Required packages:
    ```
    make
    gcc
    git
    ```
- Linux:
  - Debian:
  ```bash
  sudo apt-get install libsdl2-2.0-0 libsdl2-ttf-2.0-0 libsdl2-image-2.0-0 libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
  ```
  - Fedora:
  ```bash
  sudo dnf install SDL2 SDL2-devel SDL2_image SDL2_image-devel SDL2_ttf SDL2_ttf-devel
  ```
  - Other distros:
    Check the repos for the libraries and the headers. If not available, compile from source.
    [SDL2 Wiki](https://wiki.libsdl.org/SDL2/Installation)

- Windows:
  - MinGW:
    Get the latest [SDL2](https://github.com/libsdl-org/SDL/releases/latest) (download `SDL2-<version>-win32-x64.zip` and `SDL2-devel-<version>-mingw.zip`).
    Get the latest [SDL2_image](https://github.com/libsdl-org/SDL_image/releases/latest) (download `SDL2_image-<version>-win32-x64.zip` and `SDL2_image-devel-<version>-mingw.zip`).
    Get the latest [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf/releases/latest) (download `SDL2_ttf-<version>-win32-x64.zip` and `SDL2_ttf-devel-<version>-mingw.zip`).

    Make sure you've downloaded the version `2` files and not the version `1` or `3`.

    Now, the `-devel-` files contain the headers to be used with the libraries. Extract the non `-devel-` files somewhere, then copy the `.dll` files over 
    at the root of the project.

    Extract the `-devel-` files somewhere, `cd` in until you see a `Makefile` and do `make native` for each of the library folders (SDL2, _image, _ttf).
    This will install the headers at `/usr`. (FIXME: why install the headers and not the libraries?)

## Building

At this point you should have all the libraries and headers required, so just go to the root of the project and run `make` to generate a `game` file 
for Linux or a `game.exe` for Windows.

Run and enjoy!

## Game Control:
- **A / LeftArrow**: For moving character to the left.
- **D / RightArrow**: For Moving character to the right.
- **W / Space / UpArrow**: For using the zet of character.
- **K**: Debug Mode (Currently Shows Red overlay over the entities)
- **F**: Shoot Bullet.

## Contributors & Helpers:
- [9x14s](https://github.com/9x14S):
  - Making the game Windows Compatible.
  - Provided invaluable debugging support and feedback
