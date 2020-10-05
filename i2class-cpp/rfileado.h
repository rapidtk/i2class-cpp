#ifndef RFILEADO_H
#define RFILEADO_H

#include <ADOdb.hpp>

#include "rfile.h"

#define copyString(a, b) a=getAnsiString(b).c_str()
#define setValue(a, b)
#define setText(a, b)

class RrecordADO : public Rrecord
{
friend class RfileADO;

public:
	char getChar(int columnNumber);
	double getDouble(int columnNumber);
	int	getInt(int columnNumber);
	AnsiString getAnsiString(int columnNumber);

protected:
	int	keyLength, keyCount;
   virtual Variant *getVariantKey() {};
private:
	TADOTable *fp;
};

class RfileADO : public Rfile
{
public:
	RfileADO(AS400 &as400, char *sFileName);
	~RfileADO();

	char close();
	char open(const char *openType, int blockingFactor=0,
    char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
   void setRecordFormat(RrecordADO &format);
	char readx();
	char read();

	char read(RrecordADO &format);
	void update();
	void update(RrecordADO &format);
	char write();
	char write(RrecordADO &format);
	//char write(RrecordADO &format, unsigned long rrn);
	char chain();
	char chain(RrecordADO &format);
	char chain(RrecordADO &format, long rrn);

	char readxe();
   char reade();
	char reade(RrecordADO &format);
	char readEqual();
	char readp();
	char readp(RrecordADO &format);
	char readpe();
	char readpe(RrecordADO &format);
	char readpEqual();
	char setgt();
	char setgt(RrecordADO &format);
	char setgt(RrecordADO &format, long rrn);
	char setll();
	char setll(RrecordADO &format);
	char setll(RrecordADO &format, long rrn);
	char Delete();
	char Delete(RrecordADO &format);
	char Delete(RrecordADO &format, long rrn);

private:
	TADOTable *fp;
   char direction;
};
#endif
