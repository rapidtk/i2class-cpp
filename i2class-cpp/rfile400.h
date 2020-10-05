#ifndef RFILE400_H
#define RFILE400_H

#include <recio.h>
#include "xxfdbk.h"
#include "as400.h"

#define getInt(col) col
#define getDouble(col) col
#define getChar(col) col
#define setValue(col, value) col=value
#define setText(col, value) col ## _TEXT=value
#define copyString(a, b) a=b
#define getIndicator(ind) in ## ind
#define setIndicator(ind, value) in ## ind ## _TEXT=value

class Rrecord400 : public Rrecord
{
	friend class Rfile400;
public:
	Rrecord400(char *sRecordName) : Rrecord(sRecordName) {};

//protected:
	void	*inputBuffer, *outputBuffer;
	int	inputSize, outputSize;
	unsigned long	recno;
};
class RdbRecord400 : public Rrecord400, public RdbRcd
{
public:
	RdbRecord400(char *sRecordName) : Rrecord400(sRecordName) {};
};

class Rfile400 : public Rfile
{
public:
	Rfile400(const AS400 &as400, char *sFileName) : Rfile(as400, sFileName) {};
	/*
	Rfile400(const AS400 &as400, char *sFileName, Rrecord400 &format)
	 : Rfile(as400, sFileName, format) {};
	Rfile400(AS400 &as400, char *sFileName, Rrecord400 &format, char *openType);
	*/
	~Rfile400();
	char close();
	char open(const char *openType, int blockingFactor=0,
	 char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	void setRecordFormat(Rrecord400 &format);
	void feod();
	char read();
	char read(Rrecord400 &format);
	void update();
	void update(Rrecord400 &format);
	char write();
	char write(Rrecord400 &format);
	char write(Rrecord400 &format, unsigned long rrn);
	char chain();
	char chain(Rrecord400 &format);
	char chain(Rrecord400 &format, long rrn);


protected:
	_RFILE	*fp;
	_RIOFB_T	*feedBack;
	char readx();
};

class RdbFile400 : public Rfile400
{
public:
	RdbFile400(const AS400 &as400, char *sFileName):Rfile400(as400, sFileName){};
   /*
	RdbFile400(AS400 &as400, char *sFileName, Rrecord400 &format)
	 : Rfile400(as400, sFileName, format) {};
   */
	char reade();
	char reade(Rrecord400 &format);
	char readEqual();
	char readp();
	char readp(Rrecord400 &format);
	char readpe();
	char readpe(Rrecord400 &format);
	char readpEqual();
	char setgt();
	char setgt(Rrecord400 &format);
	char setgt(Rrecord400 &format, long rrn);
	char setll();
	char setll(Rrecord400 &format);
	char setll(Rrecord400 &format, long rrn);
	char Delete();
	char Delete(Rrecord400 &format);
	char Delete(Rrecord400 &format, long rrn);

	static void commit(AS400 system, char *cmtID="RIO");
	static void rolbk(AS400 system);
	void unlock();
private:
	bool locate();
	char readxe();
	char setgtEOF();
};

class RfileDspf : public Rfile400
{
public:
	RfileDspf(AS400 &as400, char *sFileName):Rfile400(as400, sFileName){};
   /*
   RfileDspf(AS400 &as400, char *sFileName, Rrecord400 &format)
	 : Rfile400(as400, sFileName, format) {};
   */
	char open(const char *openType, int blockingFactor=0, char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	void exfmt();
	void exfmt(Rrecord400 &format);
	char readc();
	char readc(Rrecord400 &format);

	void acq(char *dev);
	void rel(char *dev);
protected:
	char readx();
};

////////////////////////////////////////////////////////////////////////////////
// Print file stuff
class RrecordOspec : public RrecordPrint
{
friend class RfileOspec;
public:
	RrecordOspec() : page(1) {}
	void flush();
	void space(int rows=1);
	void skip(int rows);

public:
	int	page;
private:
	_RFILE	*fp;
private:
	void updatePage();
};

class RfilePrtf : public Rfile400
{
public:
	RfilePrtf(AS400 &as400, char *sFileName) : Rfile400(as400, sFileName) {}
	char open(const char *openType, int blockingFactor=0, char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	char write();
protected:
	int	overflow;
	int	outputSize;
protected:
	char	checkOverflow();
	char openPrtf(const char *openType, int blockingFactor, char commitLockLevel);
private:
	_XXOPFB_T *opfb;
};
class RfileOspec : public RfilePrtf
{
public:
	RfileOspec(AS400 &as400, char *sFileName, int rcdSiz)
	 : RfilePrtf(as400, sFileName), outputSize(rcdSiz) {}
	char open(const char *openType, int blockingFactor=0, char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	void setRecordFormat(RrecordOspec &format);
	char write();
	char write(RrecordOspec &format);
	char close();
public:
	int	page;
	int	outputSize;
};
#endif // RFILE400_H
