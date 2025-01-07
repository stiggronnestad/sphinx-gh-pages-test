#!/bin/bash

# Generate doxygen documentation (XML)
echo "Generating documentation..."
doxygen doxyfile-boost-converter
doxygen doxyfile-ccu
doxygen doxyfile-core
doxygen doxyfile-dcdc
doxygen doxyfile-device
doxygen doxyfile-inverter

# Run python injector
#echo "Injecting doxygen XML files..."
python3 injector.py ../boost-converter ../ccu ../libs/core ../dcdc ../libs/device ../inverter

# Generate .png from .dot files
generatePngFromDot() {
    local project="$1"
    local directory="$2"
    echo "Generating .png files from .dot files in $directory..."
    for file in "${directory}"/*.dot; do
        # Check if the glob actually matched any files
        [ -e "$file" ] || continue
        # Extract the filename from the path
        filename="${file##*/}"

        # Remove the extension
        filename_no_ext="${filename%.*}"

        echo "Processing $file..."
        dot -Tpng "$file" -o source/_static/"${project}"/"${filename_no_ext}.png"
    done
}

# Generate png from dot files
echo "Generating .png files from .dot files..."
generatePngFromDot boost-converter source/_dot/boost-converter
generatePngFromDot ccu source/_dot/ccu
generatePngFromDot core source/_dot/core
generatePngFromDot dcdc source/_dot/dcdc
generatePngFromDot device source/_dot/device
generatePngFromDot inverter source/_dot/inverter

# Generate HTML documentation
echo "Generating HTML documentation..."
make clean
make html
