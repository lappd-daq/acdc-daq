# acdc-daq
ISU testbench code for interfacing with Eric Oberla's PSEC4 electronics. The code is meant to be used as an interface for use in other dac systems.

## Prerequisites
This code is built in c++ and requires g++ and make to build. In addition, it relies on the following libraries
* libusb
* zlib1g

To install these, issue the following command (or similar for your flavor of linux)
```
sudo apt-get install libusb-dev zlib1g-dev
```

## Features



## Usage
For exampels of the usage, look at the files under the tests directory. Each one of these is a script showing functionality of the interface.
```
