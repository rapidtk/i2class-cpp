#ifndef RFILE_H
#define RFILE_H

#include "i2400.h"
#include "as400.h"
#include "rpgtypes.h"
extern const char *READ_ONLY;
extern const char *READ_WRITE;
extern const char *WRITE_ONLY;

extern const char COMMIT_LOCK_LEVEL_NONE;
extern const char COMMIT_LOCK_LEVEL_DEFAULT;

constexpr int MAX_PRINT_FILE_WIDTH = 378;

/// @file rfile.h
/// @brief Classes for record-oriented (as opposed to stream-oriented) file access 
/// designed around the [ILE C Record I⁄O functions](https://www.ibm.com/docs/en/i/7.5.0?topic=files-ile-c-record-io-functions#recfiles).
/// The `Rxxx` classes here provide a C++ interface to the ILE C `_Rxxx()` functions and `_RFILE` data type.


/// @brief A base record format class for record-level file access
class Rrecord
{
friend class Rfile;
public:
	Rrecord() {}
	Rrecord(char *rcdName)
		{ recordName=rcdName; }
	virtual void	input(){};
	virtual void	output(){};

//protected:
	char *recordName{};
};
/// @brief A record format class for [keyed](https://www.ibm.com/docs/en/i/7.5.0?topic=rdr-reading-database-records-using-keyed-sequence-access-path)
/// record-level access.
class RdbRcd
{
public:
	void	*key;
	int	keyLength, keyCount;
};

/// @brief A base record-oriented file access class
class Rfile
{
public:
	Rfile(const AS400 &as400, char *sFileName);
   ~Rfile();
	//Rfile(const AS400 &as400, char *sFileName, Rrecord &format);
	virtual char close() {return '0';};
	virtual char open(const char *openType, int blockingFactor=0,
	 char commitLockLevel=COMMIT_LOCK_LEVEL_NONE) {return '0';};

public:
	char	error{0}, found{0}, eof{0};

//protected:
	void setRecord(Rrecord &format);
protected:
	char		*fileName;
	//char		fileName[255];
	char		*server, *password, *usrid;
	//char		server[255];
	Rrecord	*record{};
};

////////////////////////////////////////////////////////////////////////////////
/// @brief A base record format class for printed output
class RrecordPrint : public Rrecord
{
public:
	RrecordPrint() : maxColumn(0), column(1), outputSize(sizeof(outputBuffer-1)) {}
	//static void edit(char *buf, const ZonedTemp &n, const char *edtWrd);
	static void edit(char *buf, const _ConvertDecimal &n, const char *edtWrd);
	void print(const FixedTemp &f, int col=0);
	void print(const char *str, int col=0);
#ifndef NO_FUNCTION_TEMPLATE
	template <int fsz> inline void print(const Fixed<fsz> &fStr, int col=0)
		{ printChar(fStr.overlay, fsz, col); }
#endif
	//void print(const ZonedTemp &n, int col=0, const char *edtWrd=NULL);
   //void print(int i, int col=0, const char *edtWrd=NULL);
	void print(const _ConvertDecimal &n, int col=0, const char *edtWrd=NULL);
	//void print(const ZonedTemp &n, int col, char edtCde, char fillChar=' ');
	//void print(int i, int col, char edtCde, char fillChar=' ');
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
	char	outputBuffer[MAX_PRINT_FILE_WIDTH+1]{}; // Record buffer with room for first character form control character
	int	outputSize;
	int	maxColumn;
	int	column, row{};

private:
	void	printChar(const char *str, int edtLen, int col);
};
#endif // RFILE_H

