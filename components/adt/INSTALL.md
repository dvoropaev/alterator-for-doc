# Building and installing on ALT-Linux

# Build dependencies

On ALT-Linux distributions all build dependencies can be installed with following command:

```
apt-get install cmake gcc-c++ qt5-base-devel qt5-tools-devel qt5-base-common boost-devel
```

# Building with CMake and Make

```
mkdir -p build
cmake -B build .

cd build && make -j `nproc`
```

