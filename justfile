# build using make - debugging enabled
build:
 make clean
 make -j17

# flash using pyocd
flash:
	pyocd flash -t STM32F103C6 build/sin2cos2.hex

# generate documentation using doxygen
docs:
  doxygen
