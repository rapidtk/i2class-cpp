#ifndef XPORT_H
#define XPORT_H

#include "xport400.h"
#include "as400.h"
#include "rpgtypes.h"

#define XFILE400 1
#define XFILEODBC 2

// For AS/400, assume native access
#if defined(__OS400__) && !defined(XFILETYPE)
# define XFILETYPE XFILE400
#endif

#ifndef XFILETYPE
# define XFILETYPE XFILEODBC
#endif


// Include appropriate access header file
#include "Xfile.h"
#if (XFILETYPE==XFILE400)
# define XFILE XdbFile400
# define XFILE_PRINT XfileOspec
# define XRECORD_PRINT XrecordOspec
# include "Xfile400.h"
#else
# include "XfileODBC.h"
# define XFILE XfileODBC
# define XFILE_PRINT XfileLPT
# define XRECORD_PRINT XrecordLPT
#endif

#include <xxdtaa.h>
#include "xGlobal.h"
#endif

