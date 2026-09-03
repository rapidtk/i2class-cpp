# remote
SUBDIRS = src

# Demo/showcase program (see i2class-cpp.cpp) -- lets test_core()/test_file() be compiled
# and run directly on IBM i, binding against the I2CLASS *SRVPGM built in src/Rules.mk.
# Object name can't be "I2CLASS-CPP" (hyphen isn't valid in a QSYS object name, and it's
# 11 characters -- one over the 10-character MODULE/PGM name limit).
I2CLASSCPP.MODULE: i2class-cpp.cpp
I2CLASSCPP.MODULE: private OPTION := *EVENTF *SHOWUSR
I2CLASSCPP.MODULE: private ADHOCCRTFLAGS += LANGLVL(*EXTENDED0X)

# STDLIB.MODULE, RFILE.MODULE and RFILE400.MODULE are defined in the compat/, file/ and file/ibmi Rules.mk, respectively.
I2CLASS.SRVPGM: AS400.MODULE RPGTYPES.MODULE STDLIB.MODULE RFILE.MODULE RFILE400.MODULE
	@echo "=== Creating service program I2CLASS with EXPORT(*ALL)"
	@system "CHGCURLIB $(OBJLIB)"
	@echo "CRTSRVPGM SRVPGM($(OBJLIB)/I2CLASS) MODULE(AS400 RPGTYPES STDLIB RFILE RFILE400) EXPORT(*ALL) ACTGRP(*CALLER)"
	@system "CRTSRVPGM SRVPGM($(OBJLIB)/I2CLASS) MODULE(AS400 RPGTYPES STDLIB RFILE RFILE400) EXPORT(*ALL) ACTGRP(*CALLER)"


I2CLASS.PGM: I2CLASSCPP.MODULE I2CLASS.SRVPGM
	@echo "=== Creating program I2CLASS bound to service program I2CLASS"
	@echo "CRTPGM PGM($(OBJLIB)/I2CLASSCPP) MODULE($(OBJLIB)/I2CLASSCPP) BNDSRVPGM($(OBJLIB)/I2CLASS) ACTGRP(*NEW) OPTION(*DUPPROC)"
	@system "CRTPGM PGM($(OBJLIB)/I2CLASSCPP) MODULE($(OBJLIB)/I2CLASSCPP) BNDSRVPGM($(OBJLIB)/I2CLASS) ACTGRP(*NEW) OPTION(*DUPPROC)"
