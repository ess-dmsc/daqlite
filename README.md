# daqlite
This is Daquiri light - a much simplified version of Daquiri.

daqlite subscribes to Kafka topics and provides simple visualizations, such as

* Single 2D detector image
* Three 2D projections of a 3D detector image (xy, xz, yz)
* Line/bar plots of Time Of Flight (TOF) or similar

## Build

    mkdir build
    cd build
    cmake ..
    make

### Qt library
Note that the Qt6 library needs to be installed. Ensure that `qmake` is in your
path, and CMake will use this to determine the location of the Qt6 installation.

## Run

Daqlite needs a configuration file specified by the -f option

    daqlite -f myconfig.json

See examples of how to run multiple daqlite instances in the scripts/ folder and
examples of config files for different instruments in configs/
