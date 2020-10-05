#ifndef I2CLASS_H
#define I2CLASS_H

#include "i2400.h"
#include "as400.h"
#include "rpgtypes.h"

#define RFILE400 1
#define RFILEODBC 2
#define RFILEADO 3

// For AS/400, assume native access
#if defined(__OS400__) && !defined(RFILETYPE)
# define RFILETYPE RFILE400
#else
# if defined(__BCPLUSPLUS__) && !defined(NO_ADO)
#  define RFILETYPE RFILEADO
# endif
#endif

#ifndef RFILETYPE
# define RFILETYPE RFILEODBC
#endif


// Include appropriate access header file
#define NO_ADO
#include "rfile.h"
#if (RFILETYPE==RFILE400)
# define RFILE RdbFile400
# define RFILE_PRINT RfileOspec
# define RRECORD_DATABASE RdbRecord400
# define RRECORD Rrecord400
# define RRECORD_PRINT RrecordOspec
# include "rfile400.h"
#else
# include "rfileLPT.h"
# define RFILE_PRINT RfileLPT
# define RRECORD_PRINT RrecordLPT
# if (RFILETYPE==RFILEADO)
#  include "rfileADO.h"
#  define RFILE RfileADO
#  define RRECORD_DATABASE RrecordADO
#  define RRECORD RrecordADO
#  undef NO_ADO
# else
#  include "rfileodbc.h"
#  define RFILE RfileODBC
#  define RRECORD_DATABASE RrecordODBC
#  define RRECORD RrecordODBC
# endif
#endif

#include <xxdtaa.h>
#include "rglobal.h"
#endif

