# Verlet Multi thread

![image](images/image_1.png)

## Compilation

[SFML](https://www.sfml-dev.org/) and [CMake](https://cmake.org/) need to be installed.

Create a `build` directory

```sh
mkdir build
cd build
```

**Configure** and **build** the project

```sh
cmake ..
cmake --build .
```

On **Windows** it will build in **debug** by default. To build in release you need to build using this command

```sh
camke --build . --config Release
```

You will also need to add the `res` directory and the SFML dlls in the Release or Debug directory for the executable to run.

