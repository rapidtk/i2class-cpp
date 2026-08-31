# remote
SUBDIRS = file compat file/ibmi

# CRTCPPMOD uses CPPMOD_OPTION; LANGLVL is passed through the ad-hoc flags hook.
CPPMOD_OPTION := *EVENTF *SHOWUSR
ADHOCCRTFLAGS += LANGLVL(*EXTENDED0X)

I2CLASS_MODULES := AS400.MODULE RPGTYPES.MODULE STDLIB.MODULE RFILE.MODULE RFILE400.MODULE

AS400.MODULE: as400.cpp
# Have to explicitly add this here because global assignments above don't seem to work...
AS400.MODULE: private OPTION := *EVENTF *SHOWUSR
AS400.MODULE: private ADHOCCRTFLAGS += LANGLVL(*EXTENDED0X)
RPGTYPES.MODULE: RPGTypes.cpp

# STDLIB.MODULE, RFILE.MODULE and RFILE400.MODULE are defined in the compat/, file/ and file/ibmi Rules.mk, respectively.
I2CLASS.SRVPGM: $(I2CLASS_MODULES)
	@echo "=== Creating service program I2CLASS with EXPORT(*ALL)"
	@system "CRTSRVPGM SRVPGM($(OBJLIB)/I2CLASS) MODULE(AS400 RPGTYPES STDLIB RFILE RFILE400) EXPORT(*ALL) ACTGRP(*CALLER)"
