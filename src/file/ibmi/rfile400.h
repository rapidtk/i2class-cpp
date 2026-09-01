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
	Rrecord400(const char *sRecordName) : Rrecord(sRecordName) {};

//protected:
	void	*inputBuffer, *outputBuffer;
	int	inputSize, outputSize;
	unsigned long	recno;
};
class RdbRecord400 : public Rrecord400, public RdbRcd
{
public:
	RdbRecord400(const char *sRecordName) : Rrecord400(sRecordName) {};
};

class Rfile400 : public Rfile
{
public:
	Rfile400(const AS400 &as400, const char *sFileName) : Rfile(as400, sFileName) {};
	/*
	Rfile400(const AS400 &as400, char *sFileName, Rrecord400 &format)
	 : Rfile(as400, sFileName, format) {};
	Rfile400(AS400 &as400, char *sFileName, Rrecord400 &format, char *openType);
	*/
	~Rfile400();
	void close();
	void open(const char *openType, int blockingFactor=0,
	 char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	void setRecordFormat(Rrecord400 &format);
	void feod();
	bool read();
	bool read(Rrecord400 &format);
	void update();
	void update(Rrecord400 &format);
	bool write();
	bool write(Rrecord400 &format);
	bool write(Rrecord400 &format, unsigned long rrn);
	bool chain();
	bool chain(Rrecord400 &format);
	bool chain(Rrecord400 &format, long rrn);


protected:
	_RFILE	*fp;
	_RIOFB_T	*feedBack;
	bool readx();
};

class RdbFile400 : public Rfile400
{
public:
	RdbFile400(const AS400 &as400, const char *sFileName):Rfile400(as400, sFileName){};
   /*
	RdbFile400(AS400 &as400, char *sFileName, Rrecord400 &format)
	 : Rfile400(as400, sFileName, format) {};
   */
	bool reade();
	bool reade(Rrecord400 &format);
	bool readEqual();
	bool readp();
	bool readp(Rrecord400 &format);
	bool readpe();
	bool readpe(Rrecord400 &format);
	bool readpEqual();
	bool setgt();
	bool setgt(Rrecord400 &format);
	bool setgt(Rrecord400 &format, long rrn);
	bool setll();
	bool setll(Rrecord400 &format);
	bool setll(Rrecord400 &format, long rrn);
	bool Delete();
	bool Delete(Rrecord400 &format);
	bool Delete(Rrecord400 &format, long rrn);

	static void commit(AS400 system, const char *cmtID="I2CLASS");
	static void rolbk(AS400 system);
	void unlock();
private:
	bool locate();
	bool readxe();
	bool setgtEOF();
};

class RfileDspf : public Rfile400
{
public:
	RfileDspf(AS400 &as400, const char *sFileName):Rfile400(as400, sFileName){};
   /*
   RfileDspf(AS400 &as400, char *sFileName, Rrecord400 &format)
	 : Rfile400(as400, sFileName, format) {};
   */
	void open(const char *openType, int blockingFactor=0, char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	void exfmt();
	void exfmt(Rrecord400 &format);
	bool readc();
	bool readc(Rrecord400 &format);

	void acq(const char *dev);
	void rel(const char *dev);
protected:
	bool readx();
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
	RfilePrtf(AS400 &as400, const char *sFileName) : Rfile400(as400, sFileName) {}
	void open(const char *openType, int blockingFactor=0, char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	bool write();
protected:
	int	overflow;
	int	outputSize;
protected:
	bool	checkOverflow();
	void openPrtf(const char *openType, int blockingFactor, char commitLockLevel);
private:
	_XXOPFB_T *opfb;
};
class RfileOspec : public RfilePrtf
{
public:
	RfileOspec(AS400 &as400, const char *sFileName, int rcdSiz)
	 : RfilePrtf(as400, sFileName), outputSize(rcdSiz) {}
	void open(const char *openType, int blockingFactor=0, char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	void setRecordFormat(RrecordOspec &format);
	bool write();
	bool write(RrecordOspec &format);
	void close();
public:
	int	page;
	int	outputSize;
};
#endif // RFILE400_H
