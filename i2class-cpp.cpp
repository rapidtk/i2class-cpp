// i2class-cpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/// @file i2class-cpp.cpp
/// @brief Demo/manual smoke test exercising the core RPG-compatible data types.
///
/// Illustrates the IBM i/RPG data type wrappers this library provides:
///  - Fixed<N>: a fixed-length character field (like RPG CHAR(N)); comparisons
///    are blank-padded, matching RPG semantics rather than C-string semantics.
///  - Zoned<P,F>: a zoned-decimal numeric field with P total digits and F
///    digits after the decimal point (see xxcvt.cpp for the on-disk format).
///  - packed(P,F): a packed (BCD) decimal field - same numeric semantics as
///    Zoned but stored two digits per byte, as IBM i COBOL/RPG PACKED fields are.
///  - Figurative constants ZEROS/BLANKS/LOVAL/HIVAL mirror RPG's *ZEROS,
///    *BLANKS, *LOVAL, and *HIVAL special values.

#include <iostream>
#include <string>

#include "i2class.h"

// I2CLASS_TESTS_DIR is set by CMakeLists.txt to the absolute path of tests/ (where
// CUSTMAST.csv lives); falls back to a relative path if built outside CMake.
#ifndef I2CLASS_TESTS_DIR
#define I2CLASS_TESTS_DIR "tests"
#endif
// Where the generated CUSTMAST.db lives (non-Windows only) -- the build directory, since
// it is a build artifact rather than a source fixture.
#ifndef I2CLASS_DB_DIR
#define I2CLASS_DB_DIR I2CLASS_TESTS_DIR
#endif

void test_file() {
#if RFILETYPE == RFILE400
   std::string connStr = "localhost";
   constexpr const char *CUSTMAST_FILE_NAME = "CUSTMAST";
#elif RFILETYPE == RFILEADO
   std::string connStr = "localhost";
   constexpr const char *CUSTMAST_FILE_NAME = "CUSTMAST.csv";
#elif defined(_WIN32)
   // Windows has a built-in CSV/text ODBC driver, so tests/CUSTMAST.csv can be queried
   // where it sits. AS400's single-string constructor passes this straight through to
   // SQLDriverConnect (see RfileODBC::open()); the Text driver's table name is the
   // file name including its extension.
   std::string connStr = std::string("Driver={Microsoft Access Text Driver (*.txt, *.csv)};Dbq=")
      + I2CLASS_TESTS_DIR + ";Extensions=asc,csv,tab,txt;HDR=Yes;FMT=Delimited;";
   constexpr const char *CUSTMAST_FILE_NAME = "CUSTMAST.csv";
#else
   // unixODBC ships no text/CSV driver, so on Linux the same rows are read from a SQLite
   // database generated from CUSTMAST.csv at build time (needs the libsqliteodbc package).
   std::string connStr = std::string("Driver=SQLite3;Database=")
      + I2CLASS_DB_DIR + "/CUSTMAST.db;";
   constexpr const char *CUSTMAST_FILE_NAME = "CUSTMAST";
#endif

   AS400 as400(connStr.c_str());

   typedef zoned(7, 2) ORDVAL_T;
   static ORDVAL_T ordval;

   /* The custmast.h header file would be created through a tool analogous to GENSRC but I2CLASS specific.
   *  Included inline here for demo
#include "custmast.h"
   */
   class CUSTMAST_CUSFMT : public RRECORD {
   public:
#if RFILETYPE == RFILE400
#pragma pack(1)
		zoned(6, 0) CUSNO;
		fixed(20) CNAME;
		ORDVAL_T ORDVAL;
#pragma pack()
	  CUSTMAST_CUSFMT() : RRECORD("CUSTMAST") {
		 inputBuffer = &CUSNO;
		 inputSize = 33;
	  }
#else
	   enum FieldList {
		  CUSNO = 1,
		  CNAME = 2,
		  ORDVAL = 3
	   };
#endif
	  void input () {
	     ordval = getDouble(ORDVAL);
	  }
	  double getOrdersValue() { return getDouble(ORDVAL); }
   } custmast_rcd;
   RFILE custmast(as400, CUSTMAST_FILE_NAME);

   // Accumulate total of order values from all customers
   float totalOrdersValue = 0.0;
   packed(9,2) totalOrdersValue92 = 0.0;
   custmast.setRecordFormat(custmast_rcd);
   try
   {
      custmast.open(READ_ONLY);
   }
   catch (const CI2ErrFile &err)
   {
      std::cout << "Failed to open CUSTMAST.csv: " << err.message << '\n';
      return;
   }
   while (custmast.read()) {
	  totalOrdersValue += ordval;
	  totalOrdersValue92 = totalOrdersValue92 + ordval;
   }
   custmast.close();
   printf("Total float orders value to 4 decimal points: %.4f\n", totalOrdersValue);
   std::cout << "Total packed(9,2) orders value {exact}: " << totalOrdersValue92 << '\n';
}

void test_core()
{
    Fixed<10> fxd10;
    fxd10 = "abcdef";
	std::cout << "char(10): " << fxd10.c_str() << '\n' ;
	Fixed<3> fxd3 = "abc";
	if (fxd10 != fxd3)
	   std::cout << "char(10) 'abcdef' not equal to char(3) 'abc' \n";
	if (fxd3 == "abc")
	   std::cout << "char(3) 'abc' equal to 'abc'\n";
	fxd3 = ZEROS;
	if (fxd3 == "000")
	   std::cout << "char(3) '000' equal to *ZEROS\n";

	Zoned<3, 1> znd31 = 12.1;
	std::cout << "zoned(3,1): " << (double)znd31 << '\n';
	znd31.movel('3');
	if ((double)znd31 == 32.1)
	   std::cout << "zoned(3,1) 32.1 equal to 32.1\n";
	Zoned<5, 2> znd52 = 123.45;
	double diff = (double)znd52 - 123.45;
	if (diff == 0.0)
	   std::cout <<	"zoned(5,2) 123.45 - 123.45 equal to 0.0\n";

	packed(4, 2) pkd42;
	pkd42 = znd31;
	if (pkd42 == znd31)
	   std::cout << "packed(4,2) 32.10 equal to zoned(3,1) 32.1\n";


	znd31 = LOVAL;
	if ((double)znd31 == -99.9)
	   std::cout << "zoned(3,1) -99.9 equal to *LOVAL\n";
	pkd42 = 99.99;
	if (pkd42 == HIVAL)
	   std::cout << "packed(4,2) 99.99 equal to *HIVAL\n";

	std::cout << "EDITWRD zoned(5,2) 123.45: "
		<< EDITWRD(znd52, "  0.  ") << '\n';
	std::cout << "EDITC zoned(5,2) 123.45 code 1: "
		<< EDITC(znd52, '1') << '\n';
	std::cout << "zoned member editwrd: "
		<< znd52.editwrd("  0.  ") << '\n';
	std::cout << "zoned member editc: "
		<< znd52.editc('1') << '\n';
	std::cout << "packed member editc: "
		<< pkd42.editc('1') << '\n';

	Zoned<5, 2> roundedSource = 12.35;
	Zoned<5, 1> roundedOne = roundedSource.round<1>();
	Zoned<5, 0> roundedWhole = roundedSource.round<0>();
	std::cout << "zoned round<1>(12.35): " << roundedOne << '\n';
	std::cout << "zoned round<0>(12.35): " << roundedWhole << '\n';
	std::cout << "zoned inth(12.35): " << roundedSource.inth() << '\n';
	packed(5,2) packedRounded;
	packedRounded = roundedSource;
	std::cout << "packed inth(12.35): " << packedRounded.inth() << '\n';

	std::cout << "SCANRPL: " << SCANRPL("cat", "dog", "cat scat") << '\n';
	std::cout << "REPLACE: " << REPLACE("XY", "abcdef", 3, 2) << '\n';
	std::vector<std::string> splitResult = SPLIT("a,,b,", ",");
	std::cout << "SPLIT count: " << splitResult.size() << '\n';
	std::cout << "XLATE: " << XLATE("abc", "123", "cab") << '\n';
	std::cout << "UPPER: " << UPPER("Abc 123") << '\n';
	std::cout << "LOWER: " << LOWER("AbC 123") << '\n';

}

int main()
{
	test_core();
	test_file();
}