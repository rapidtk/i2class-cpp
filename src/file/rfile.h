#ifndef RFILE_H
#define RFILE_H

#include "i2compat.h"
#include "as400.h"
#include "rpgtypes.h"
extern I2CLASS_API czstring READ_ONLY;
extern I2CLASS_API czstring READ_WRITE;
extern I2CLASS_API czstring WRITE_ONLY;

extern I2CLASS_API const char COMMIT_LOCK_LEVEL_NONE;
extern I2CLASS_API const char COMMIT_LOCK_LEVEL_DEFAULT;

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
	Rrecord() : recordName(nullptr) {}
	Rrecord(czstring rcdName)
		{ recordName=rcdName; }
	virtual ~Rrecord() = default;
	virtual void	input(){};
	virtual void	output(){};

//protected:
	czstring recordName;
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
	Rfile(const AS400 &as400, czstring sFileName);
   virtual ~Rfile();
	//Rfile(const AS400 &as400, char *sFileName, Rrecord &format);
	/// @brief Close the file. Throws CI2ErrFile on failure (base no-op never fails).
	virtual void close() {}
	/// @brief Open the file. Throws CI2ErrFile on failure (base no-op never fails).
	virtual void open(czstring /*openType*/, int /*blockingFactor*/=0,
	 char /*commitLockLevel*/=COMMIT_LOCK_LEVEL_NONE) {}

public:
	bool	error, found, eof;

//protected:
	void setRecord(Rrecord &format);
protected:
	czstring		fileName;
	//char		fileName[255];
	czstring		server, password, usrid;
	//char		server[255];
	Rrecord	*record;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief A base record format class for printed output
class RrecordPrint : public Rrecord
{
public:
	RrecordPrint() : outputBuffer(), outputSize(sizeof(outputBuffer) - 1), maxColumn(0), column(1), row(0) {}
	//static void edit(char *buf, const ZonedTemp &n, const char *edtWrd);
	static void edit(zstring buf, const _ConvertDecimal &n, czstring edtWrd);
	void print(const FixedTemp &f, int col=0);
	void print(czstring str, int col=0);
#ifndef NO_FUNCTION_TEMPLATE
	template <int fsz> inline void print(const Fixed<fsz> &fStr, int col=0)
		{ printChar(reinterpret_cast<const_byte_ptr>(fStr.overlay), fsz, col); }
#endif
	//void print(const ZonedTemp &n, int col=0, const char *edtWrd=NULL);
   //void print(int i, int col=0, const char *edtWrd=NULL);
	void print(const _ConvertDecimal &n, int col=0, czstring edtWrd=NULL);
	//void print(const ZonedTemp &n, int col, char edtCde, char fillChar=' ');
	//void print(int i, int col, char edtCde, char fillChar=' ');
	void print(const _ConvertDecimal &n, int col, char edtCde, char fillChar=' ');
	void printl(const FixedTemp &f, int col=0);
	void printl(czstring str, int col=0);
	//void printl(const ZonedTemp &n, int col=0, const char *edtWrd=NULL);
	void printl(const _ConvertDecimal &n, int col=0, czstring edtWrd=NULL);
	//void printl(const ZonedTemp &n, int col, char edtCde, char fillChar=' ');
	void printl(const _ConvertDecimal &n, int col, char edtCde, char fillChar=' ');
//	void printl(Fixed &f, int col=0);
//	void printl(Numeric &n, int col=0, char *edtWrd=NULL);

protected:
	virtual void flush()=0;
	void setMaxColumn(int &col);

protected:
	//char	*outputBuffer;
	char	outputBuffer[MAX_PRINT_FILE_WIDTH+1]; // Record buffer with room for first character form control character
	int	outputSize;
	int	maxColumn;
	int	column, row;

private:
	void	printChar(const_byte_ptr str, int edtLen, int col);
};
#endif // RFILE_H

