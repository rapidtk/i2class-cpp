#ifndef XFILE_H
#define XFILE_H

#include "xport400.h"
#include "as400.h"
#include "rpgtypes.h"
extern const char *READ_ONLY;
extern const char *READ_WRITE;
extern const char *WRITE_ONLY;

extern const char COMMIT_LOCK_LEVEL_NONE;
extern const char COMMIT_LOCK_LEVEL_DEFAULT;

class Xrecord
{
friend class Xfile;
public:
	Xrecord() {}
	Xrecord(char *rcdName)
		{ recordName=rcdName; }
	virtual void	input(){};
	virtual void	output(){};

//protected:
	char	*recordName;
};
class XdbRcd
{
public:
	void	*key;
	int	keyLength;
};

class Xfile
{
public:
	Xfile(const AS400 &as400, char *sFileName);
	//Xfile(const AS400 &as400, char *sFileName, Xrecord &format);
	virtual char close() {return '0';};
	virtual char open(const char *openType, int blockingFactor=0,
	 char commitLockLevel=COMMIT_LOCK_LEVEL_NONE) {return '0';};

public:
	char	error, noData;

//protected:
	void setRecord(Xrecord &format);
protected:
	char		*fileName;
	//char		fileName[255];
	char		*server;
	//char		server[255];
	Xrecord	*record;
};

////////////////////////////////////////////////////////////////////////////////
// Print file stuff
class XrecordPrint : public Xrecord
{
public:
	XrecordPrint() : maxColumn(0), column(1), outputSize(sizeof(outputBuffer-1)) {}
	//static void edit(char *buf, const ZonedTemp &n, const char *edtWrd);
	static void edit(char *buf, const _ConvertDecimal &n, const char *edtWrd);
	void print(const FixedTemp &f, int col=0);
	void print(const char *str, int col=0);
#ifndef NO_FUNCTION_TEMPLATE
	template <int fsz> inline void print(const Fixed<fsz> &fStr, int col=0)
		{ printChar(fStr.overlay, fsz, col); }
#endif
	//void print(const ZonedTemp &n, int col=0, const char *edtWrd=NULL);
	void print(int i, int col=0, const char *edtWrd=NULL);
	void print(const _ConvertDecimal &n, int col=0, const char *edtWrd=NULL);
	//void print(const ZonedTemp &n, int col, char edtCde, char fillChar=' ');
	void print(int i, int col, char edtCde, char fillChar=' ');
	void print(const _ConvertDecimal &n, int col, char edtCde, char fillChar=' ');
	void printl(const FixedTemp &f, int col=0);
	void printl(const char *str, int col=0);
	//void printl(const ZonedTemp &n, int col=0, const char *edtWrd=NULL);
	void printl(const _ConvertDecimal &n, int col=0, const char *edtWrd=NULL);
	//void printl(const ZonedTemp &n, int col, char edtCde, char fillChar=' ');
	void printl(const _ConvertDecimal &n, int col, char edtCde, char fillChar=' ');
//	void printl(Fixed &f, int col=0);
//	void printl(Numeric &n, int col=0, char *edtWrd=NULL);

protected:
	virtual void flush()=0;
	void setMaxColumn(int &col);

protected:
	//char	*outputBuffer;
	char	outputBuffer[379]; // Record buffer with room for first character form control character
	int	outputSize;
	int	maxColumn;
	int	column, row;

private:
	void	printChar(const char *str, int edtLen, int col);
};
#endif // XFILE_H

