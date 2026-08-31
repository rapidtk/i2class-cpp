// Regression test for RfileODBC/RrecordODBC against tests/CUSTMAST.csv (see i2class-cpp.cpp's
// test_file() for the "showcase sample" this mirrors). Requires ODBC + the Windows-builtin
// Microsoft Text Driver -- only built/registered when CMakeLists.txt's find_package(ODBC)
// succeeds.
#include <cstdio>
#include <string>

#include "i2class.h"

#ifndef I2CLASS_TESTS_DIR
#define I2CLASS_TESTS_DIR "tests"
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
};

int main()
{
	std::string connStr = std::string("Driver={Microsoft Access Text Driver (*.txt, *.csv)};Dbq=")
		+ I2CLASS_TESTS_DIR + ";Extensions=asc,csv,tab,txt;HDR=Yes;FMT=Delimited;";
	AS400 as400(connStr.c_str());
	CustmastRecord rec;
	RFILE custmast(as400, "CUSTMAST.csv");

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

	if (failures == 0)
	{
		std::printf("All CUSTMAST.csv checks passed (total=%.2f).\n", total);
		return 0;
	}
	std::printf("%d CUSTMAST.csv check(s) failed (total=%.2f).\n", failures, total);
	return 1;
}
