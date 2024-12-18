#!/bin/bash

# Generate doxygen documentation (XML)
doxygen doxyfile-boost-converter
doxygen doxyfile-ccu
doxygen doxyfile-core
doxygen doxyfile-dcdc
doxygen doxyfile-device
doxygen doxyfile-inverter

# Run python injector
python3 injector.py ../boost-converter ../ccu ../libs/core ../dcdc ../libs/device ../inverter

# Generate HTML documentation
make clean
make html