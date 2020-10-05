#ifndef AS400FILE_H
#define AS400FILE_H

#include <recio.h>
#include "xxfdbk.h"

#define getInt(col) col
#define getDouble(col) col
#define fixedCpy(colto, colfrom) colto=colfrom

class Xrecord400 : public Xrecord
{
	friend class Xfile400;
public:
	Xrecord400(char *sRecordName) : Xrecord(sRecordName) {};

//protected:
	void	*inputBuffer, *outputBuffer;
	int	inputSize, outputSize;
	unsigned long	recno;
};
class XdbRecord400 : public Xrecord400, public XdbRcd
{
public:
	XdbRecord400(char *sRecordName) : Xrecord400(sRecordName) {};
};

class Xfile400 : public Xfile
{
public:
	Xfile400(const AS400 &as400, char *sFileName) : Xfile(as400, sFileName) {};
	/*
	Xfile400(const AS400 &as400, char *sFileName, Xrecord400 &format)
	 : Xfile(as400, sFileName, format) {};
	Xfile400(AS400 &as400, char *sFileName, Xrecord400 &format, char *openType);
	*/
	~Xfile400();
	char close();
	char open(const char *openType, int blockingFactor=0,
	 char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	void setRecordFormat(Xrecord400 &format);
	void feod();
	char read();
	char read(Xrecord400 &format);
	void update();
	void update(Xrecord400 &format);
	char write();
	char write(Xrecord400 &format);
	char write(Xrecord400 &format, unsigned long rrn);
	char chain();
	char chain(Xrecord400 &format);
	char chain(Xrecord400 &format, long rrn);


protected:
	_RFILE	*fp;
	_RIOFB_T	*feedBack;
	char readx();
};

class XdbFile400 : public Xfile400
{
public:
	XdbFile400(const AS400 &as400, char *sFileName):Xfile400(as400, sFileName){};
	/*
	XdbFile400(AS400 &as400, char *sFileName, Xrecord400 &format)
	 : Xfile400(as400, sFileName, format) {};
	*/
	char reade();
	char reade(Xrecord400 &format);
	char readp();
	char readp(Xrecord400 &format);
	char readpe();
	char readpe(Xrecord400 &format);
	char setgt();
	char setgt(Xrecord400 &format);
	char setgt(Xrecord400 &format, long rrn);
	char setll();
	char setll(Xrecord400 &format);
	char setll(Xrecord400 &format, long rrn);
	char Delete();
	char Delete(Xrecord400 &format);
	char Delete(Xrecord400 &format, long rrn);

	static void commit(char *cmtID=NULL);
	static void rolbk();
	void unlock();
private:
	bool locate();
	char readxe();
	char setgtEOF();
};

class XfileDspf : public Xfile400
{
public:
	XfileDspf(AS400 &as400, char *sFileName):Xfile400(as400, sFileName){};
	char open(const char *openType, int blockingFactor=0, char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	void exfmt();
	void exfmt(Xrecord400 &format);
	char readc();
	char readc(Xrecord400 &format);

	void acq(char *dev);
	void rel(char *dev);
};

////////////////////////////////////////////////////////////////////////////////
// Print file stuff
class XrecordOspec : public XrecordPrint
{
friend class XfileOspec;
public:
	XrecordOspec() : page(1) {}
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

class XfilePrtf : public Xfile400
{
public:
	XfilePrtf(AS400 &as400, char *sFileName) : Xfile400(as400, sFileName) {}
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
class XfileOspec : public XfilePrtf
{
public:
	XfileOspec(AS400 &as400, char *sFileName, int rcdSiz)
	 : XfilePrtf(as400, sFileName), outputSize(rcdSiz) {}
	char open(const char *openType, int blockingFactor=0, char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	void setRecordFormat(XrecordOspec &format);
	char write();
	char write(XrecordOspec &format);
	char close();
public:
	int	page;
	int	outputSize;
};
#endif // AS400FILE_H
