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
	RfileADO(AS400 &as400, czstring sFileName);
	~RfileADO();

	void close();
	void open(czstring openType, int blockingFactor=0,
    char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
   void setRecordFormat(RrecordADO &format);
	bool readx();
	bool read();

	bool read(RrecordADO &format);
	void update();
	void update(RrecordADO &format);
	bool write();
	bool write(RrecordADO &format);
	//char write(RrecordADO &format, unsigned long rrn);
	bool chain();
	bool chain(RrecordADO &format);
	bool chain(RrecordADO &format, long rrn);

	bool readxe();
   bool reade();
	bool reade(RrecordADO &format);
	bool readEqual();
	bool readp();
	bool readp(RrecordADO &format);
	bool readpe();
	bool readpe(RrecordADO &format);
	bool readpEqual();
	bool setgt();
	bool setgt(RrecordADO &format);
	bool setgt(RrecordADO &format, long rrn);
	bool setll();
	bool setll(RrecordADO &format);
	bool setll(RrecordADO &format, long rrn);
	bool Delete();
	bool Delete(RrecordADO &format);
	bool Delete(RrecordADO &format, long rrn);

private:
	TADOTable *fp;
   char direction;
};
#endif
