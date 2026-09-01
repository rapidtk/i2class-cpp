// Regression test for RfileODBC/RrecordODBC against tests/CUSTMAST.csv (see i2class-cpp.cpp's
// test_file() for the "showcase sample" this mirrors). Reads the CSV directly through the
// Windows-builtin Text driver, or through the SQLite database CMake generates from it
// elsewhere -- unixODBC has no CSV driver. Only built when find_package(ODBC) succeeds.
#include <cstdio>
#include <string>

#include "i2class.h"

#ifndef I2CLASS_TESTS_DIR
#define I2CLASS_TESTS_DIR "tests"
#endif
#ifndef I2CLASS_DB_DIR
#define I2CLASS_DB_DIR I2CLASS_TESTS_DIR
#endif

#ifdef _WIN32
# define CUSTMAST_CONN_STR (std::string("Driver={Microsoft Access Text Driver (*.txt, *.csv)};Dbq=") \
	+ I2CLASS_TESTS_DIR + ";Extensions=asc,csv,tab,txt;HDR=Yes;FMT=Delimited;")
# define CUSTMAST_FILE_NAME "CUSTMAST.csv"
#else
# define CUSTMAST_CONN_STR (std::string("Driver=SQLite3;Database=") + I2CLASS_DB_DIR + "/CUSTMAST.db;")
# define CUSTMAST_FILE_NAME "CUSTMAST"
#endif

static int failures = 0;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::fprintf(stderr, "FAIL: %s (rfileodbc_custmast_check.cpp:%d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

class CustmastRecord : public RRECORD
{
public:
	enum FieldList { CUSNO = 1, CNAME = 2, ORDVAL = 3 };
	double getOrdersValue() { return readDouble(ORDVAL); }
	// Exact-decimal path (see RrecordODBC::readDecimal()) -- no binary floating-point
	// round-trip, unlike getOrdersValue() above.
	double getOrdersValueExact()
	{
		Zoned<9,2> z;
		if (!readDecimal(ORDVAL, z))
			return 0.0;
		return (double)(long double)z;
	}
};

int main()
{
	std::string connStr = CUSTMAST_CONN_STR;
	AS400 as400(connStr.c_str());
	CustmastRecord rec;
	RFILE custmast(as400, CUSTMAST_FILE_NAME);

	custmast.setRecordFormat(rec);
	bool opened = true;
	try
	{
		custmast.open(READ_ONLY);
	}
	catch (const CI2ErrFile &err)
	{
		std::fprintf(stderr, "open() threw CI2ErrFile: %s\n", err.message);
		opened = false;
	}
	CHECK(opened);

	double total = 0.0;
	int rowCount = 0;
	while (custmast.read())
	{
		total += rec.getOrdersValue();
		++rowCount;
	}
	custmast.close();

	CHECK(rowCount == 3);
	CHECK(total > 60145.75 && total < 60145.77); // 5299.99 + 50328 + 4517.77

	// Re-run using the exact-decimal path (readDecimal()/Zoned<>), in a separate open/read
	// pass -- ODBC drivers generally don't support calling SQLGetData twice for the same
	// (fixed-length) column within a single row, so this can't share the loop above.
	RFILE custmastExact(as400, CUSTMAST_FILE_NAME);
	CustmastRecord recExact;
	custmastExact.setRecordFormat(recExact);
	bool openedExact = true;
	try
	{
		custmastExact.open(READ_ONLY);
	}
	catch (const CI2ErrFile &err)
	{
		std::fprintf(stderr, "open() threw CI2ErrFile: %s\n", err.message);
		openedExact = false;
	}
	CHECK(openedExact);
	double totalExact = 0.0;
	int rowCountExact = 0;
	while (custmastExact.read())
	{
		totalExact += recExact.getOrdersValueExact();
		++rowCountExact;
	}
	custmastExact.close();

	CHECK(rowCountExact == 3);
	CHECK(totalExact > 60145.75 && totalExact < 60145.77);

	if (failures == 0)
	{
		std::printf("All CUSTMAST.csv checks passed (total=%.2f).\n", total);
		return 0;
	}
	std::printf("%d CUSTMAST.csv check(s) failed (total=%.2f).\n", failures, total);
	return 1;
}
