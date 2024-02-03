# Pg Engine

An ecs based Game/Graphical Engine

# Exemple of game possible

## Tetris clone

A full fledge tetris clone using texture, entities and sound effects using this engine

[Tetris clone](tetris.gif)

# How to build

## Download external depencies

The first step to build is to download the two external depencies of this project:
- Mingw64 (or any other c++ compiler)
- QT

The first step is to install [Mingw 64](https://www.mingw-w64.org/downloads/) 

Once mingw is installed, add the bin folder in your PATH to use make !
[Set up mingw in path](docs/mingwpath.png)

The next step is to install QT, please use [this online installer](https://www.qt.io/download-qt-installer)
Only QT open source is mendatory for this project, After the registration step install a version of QT at least equals to 5.11 !
It should be noted that this application needs to be build with the support for 64bits for QT for the newer version of QT (like the 5.15), it comes pre build with 64bits but for older version (like the 5.11) you will need to compile it yourself against mingw64!

Once the download is done, insert QTPATH pointing to your freshly built QT inside your environment variables.

[Set up qt in environment variables](docs/qtpathimage.png)

This will enable the Makefile to find QT depencies as well as utilitaries, such as rcc used to compile ressource file in the .exe and moc used to resolve signals and slots made with QT ! 

Once those depencies are resolved, you can build the application with:
- `mingw32-make DEBUG=True` in debug mode
- `mingw32-make` in release mode

Or use the different task set up for vscode

## Profiling App

To profile the Taskflow scheduling of the application, you need to enable the profiler in your environment by setting the filename in the environment variable

On windows:
- `$env:TF_ENABLE_PROFILER=simple.json`

