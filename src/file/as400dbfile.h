#ifndef AS400FILE_H
#define AS400FILE_H

#include <recio.h>

#include "as400.h"

extern const char *READ_ONLY;
extern const char *READ_WRITE;
extern const char *WRITE_ONLY;

extern const char COMMIT_LOCK_LEVEL_NONE;
extern const char COMMIT_LOCK_LEVEL_DEFAULT;

class AS400File
{
public:
   AS400File(AS400 &as400, char *sFileName);
   ~AS400File();
   char close();
   char open(const char *openType, int blockingFactor=0,
    char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
   bool setRecordFormat(char *format);

public:
   char		*fileName;
   char		*server;

protected:
	virtual void	setGlobalFields(){};
   virtual void	getGlobalFields(){};

protected:
	_RFILE	*fp;
};

class AS400dbFile : public AS400File
{
public:
   AS400dbFile(AS400 &as400, char *sFileName):AS400File(as400, sFileName){};
	char readNext();
protected:
	void	*recordBuffer;
   int	recordSize;
};
#endif // AS400FILE_H
