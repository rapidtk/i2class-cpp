SUBDIRS = file compat

# Portable core modules (mirrors the CMakeLists.txt "i2class" library sources).
AS400.MODULE: as400.cpp
RPGTYPES.MODULE: RPGTypes.cpp
XXCVT.MODULE: xxcvt.cpp
XXDTAA.MODULE: xxdtaa.cpp

# STDLIB.MODULE and RFILE.MODULE are defined in the compat/ and file/ Rules.mk.
I2CLASS.SRVPGM: I2CLASS.BND AS400.MODULE RPGTYPES.MODULE XXCVT.MODULE XXDTAA.MODULE STDLIB.MODULE RFILE.MODULE
