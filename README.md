# sin2abc

Project to emulate a three phase Hall effect sensor from a sin and cos encoder.

## Usage

- Set constants in ```generate.py``` then run script
- Rebuild project


## Design

### Lookup tables

I started off with a LUT for ATAN values given the ratio r=C/S

This will need to be modified to change from a theta value to a hall effect output, I will do that in a new branch, cause this should be tested first.
