#include "ooport.h"
//#include <string>

#include "as400file.h"

const char *READ_ONLY="rr";
const char *READ_WRITE="rr+";
const char *WRITE_ONLY="ar";

const char COMMIT_LOCK_LEVEL_NONE='n';
const char COMMIT_LOCK_LEVEL_DEFAULT='y';

AS400File::AS400File(AS400 &as400, char *sFileName)
{
   fileName = sFileName;
   server = as400.url;
}

// On deletion, close file
AS400File::~AS400File()
{
	close();
}

// Initialize the class to point to the (already allocated) structure
bool AS400File::close()
{
   _Rclose(fp);
   return true;
}

bool AS400File::open(const char *OpenType, int blockingFactor, char commitLockLevel)
{
//   string mode= (string)OpenType + ", commit=" + commitLockLevel;
//   fp = _Ropen(fileName, mode.c_str());
	fp = _Ropen(fileName, OpenType);
   return true;
}

bool AS400File::setRecordFormat(char *format)
{
	_Rformat(fp, format);
   return true;
}

bool AS400dbFile::readNext()
{
	_Rreadn(fp, recordBuffer, recordSize, __DFT);
   setGlobalFields();
   return true;
}


