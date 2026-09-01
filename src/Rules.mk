# remote
SUBDIRS = file compat file/ibmi

# CRTCPPMOD uses CPPMOD_OPTION; LANGLVL is passed through the ad-hoc flags hook.
CPPMOD_OPTION := *EVENTF *SHOWUSR
ADHOCCRTFLAGS += LANGLVL(*EXTENDED0X)

AS400.MODULE: as400.cpp
# Have to explicitly add this here because global assignments above don't seem to work...
AS400.MODULE: private OPTION := *EVENTF *SHOWUSR
AS400.MODULE: private ADHOCCRTFLAGS += LANGLVL(*EXTENDED0X)
RPGTYPES.MODULE: RPGTypes.cpp