# Klaxon Tracker

This is a tracker built in JS-EDEN, JavaScript and WebAssembly, linked together with the Web Audio API.

## Build instructions

In order to build the WebAssembly engine from scratch, clone the [Emscripten](https://github.com/emscripten-core/emscripten) repository in any directory, and follow the instructions on setting up Emscripten.
Then open a terminal with Emscripten active, and in `js-eden/klaxon-engine`.

### Windows

To build the project on Windows, first ensure that Emscripten is running in a terminal. Use `emcmake` configured with [Ninja](https://ninja-build.org/) to generate the configuration files.

If you don't have Ninja, install with Chocolatey:
``choco install Ninja``
or with winget:
``winget install -e --id Ninja-build.Ninja``

Run `emcmake` for the build files:
``
emcmake cmake -G Ninja -S . -B build
``

Then run ``cmake --build build`` to compile the project and generate a standalone WebAssembly binary.

### Linux

To build the project on Linux, first ensure that Emscripten is running in a terminal.

Run `emcmake` for the build files:
``
emcmake cmake -S . -B build
``

Then run ``cmake --build build`` to compile the project and generate a standalone WebAssembly binary.

## Running the tracker

In the `js-eden` directory, run `npm run devserver` and click on the prompted URL. Finally click on Klaxon Tracker

## Shortcuts

`Space Bar` - arm the pattern for editing

`Arrow Up, Arrow Down, Arrow Left, Arrow Right` - move through the pattern when editing

`Ctrl + Shift + (0 - 7)` - set the octave

`Q, 2, W, 3, E, R, 5, T, 6, Y, 7, U, I, 9, O, 0, P` - upper piano section (octave + 1)

`Z, S, X, D, C, V, G, B, H, N, J, M, ",", "."` - lower piano section (octave)