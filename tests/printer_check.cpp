// Regression tests for printer file support.
//
// Generates a real report through RfileLPT/RrecordLPT. On Windows the printer
// file is a local text file; on IBM i the same code path produces spooled
// output. The generated report is diffed against tests/printer-output.txt,
// which is committed so the rendering can be eyeballed and reviewed.
//
// Printer record API notes (see src/file/rfile.cpp):
//   print(x, col)  -- RIGHT justified, col is the END column
//   printl(x, col) -- LEFT justified, col is the START column
//   flush()        -- emits the accumulated line WITHOUT a newline
//   space(n)       -- flush() then emit n newlines (this is what ends a line)
//
// Tests verify:
// - print()/printl() column positioning
// - edit codes and edit words applied to zoned values
// - the emitted report matches the committed expected output
#include <cstdio>
#include <cstring>
#include <string>

#include "RPGTypes.h"
#include "rglobal.h"
#include "file/rfilelpt.h"

static int failures = 0;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::fprintf(stderr, "FAIL: %s (printer_check.cpp:%d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

////////////////////////////////////////////////////////////////////////////////
// Report data
struct SalesRow {
	czstring    item;
	czstring    description;
	int         qty;
	const char *amount;   // decimal literal, e.g. "1234.56"
};

static const SalesRow sales[] = {
	{ "A100", "Widget, Large",       12, "1234.56" },
	{ "B205", "Gear Assembly",        3,  "899.00" },
	{ "C310", "Bracket, Stainless", 250,   "17.25" },
	{ "D415", "Returned Goods",      -5, "-320.75" },
	{ "E520", "No Charge Sample",     1,    "0.00" },
};
static const int SALES_COUNT = (int)(sizeof(sales) / sizeof(sales[0]));

// Column layout: label start columns, numeric end columns.
enum {
	COL_ITEM      = 1,    // printl
	COL_DESC      = 8,    // printl
	COL_QTY_END   = 34,   // print (right justified)
	COL_AMT_END   = 50,   // print
	COL_EXT_END   = 68    // print
};

// Edit word for the Zoned<11,2> grand total: a floating '$', then 11 digit
// positions (9 before the decimal point, 2 after) with comma separators and a
// CR suffix for a credit balance. Blanks, '0' and '$' are digit positions;
// ',' '.' and "CR" are not. The '$' needs a position of its own, which is why
// there is one more than the value has digits.
#define TOTAL_EDIT_WORD " $   ,   ,  0.  CR"

////////////////////////////////////////////////////////////////////////////////
// Generate the report through the printer file
static void generate_report(czstring path)
{
	AS400 as400;
	RfileLPT prt(as400, path, MAX_PRINT_FILE_WIDTH);
	prt.open("W");

	RrecordLPT rcd;
	prt.setRecordFormat(rcd);

	// --- Page heading -------------------------------------------------------
	rcd.printl("Sales Register", 25);
	rcd.printl("Page", 60);
	rcd.print(Zoned<4, 0>(1), 68, 'Z', ' ');
	rcd.space(1);

	rcd.printl("Run Date", 1);
	// Edit code 'Y' renders a date with separators, in _DATFMT field order.
	rcd.print(UDATE, 22, 'Y', ' ');
	rcd.space(2);

	// --- Column headings ----------------------------------------------------
	rcd.printl("Item", COL_ITEM);
	rcd.printl("Description", COL_DESC);
	rcd.printl("Qty", COL_QTY_END - 2);
	rcd.printl("Amount", COL_AMT_END - 5);
	rcd.printl("Extended", COL_EXT_END - 7);
	rcd.space(1);

	rcd.printl("----", COL_ITEM);
	rcd.printl("--------------------", COL_DESC);
	rcd.printl("-----", COL_QTY_END - 4);
	rcd.printl("--------------", COL_AMT_END - 13);
	rcd.printl("--------------", COL_EXT_END - 13);
	rcd.space(1);

	// --- Detail lines -------------------------------------------------------
	Zoned<11, 2> grand_total(0);

	for (int i = 0; i < SALES_COUNT; ++i) {
		Zoned<9, 2>  amount(__D(sales[i].amount));
		Zoned<5, 0>  qty(sales[i].qty);
		Zoned<11, 2> extended = amount * qty;
		grand_total = grand_total + extended;

		rcd.printl(sales[i].item, COL_ITEM);
		rcd.printl(sales[i].description, COL_DESC);
		// 'Z' -- zero suppressed, no sign.
		rcd.print(qty, COL_QTY_END, 'Z', ' ');
		// 'J' -- commas and decimal point, trailing minus on negatives.
		rcd.print(amount, COL_AMT_END, 'J', ' ');
		rcd.print(extended, COL_EXT_END, 'J', ' ');
		rcd.space(1);
	}

	// --- Totals -------------------------------------------------------------
	rcd.space(1);
	rcd.printl("GRAND TOTAL", COL_DESC);
	rcd.print(grand_total, COL_EXT_END, TOTAL_EDIT_WORD);
	rcd.space(2);

	rcd.printl("END OF REPORT", COL_ITEM);
	rcd.space(1);

	prt.close();
}

////////////////////////////////////////////////////////////////////////////////
// Strip a line down to a comparable form: trailing blanks removed, and the
// run date masked so the committed baseline does not go stale every day.
static void normalize(char *line)
{
	size_t n = std::strlen(line);
	while (n && (line[n - 1] == '\n' || line[n - 1] == '\r' || line[n - 1] == ' '))
		line[--n] = '\0';

	if (std::strncmp(line, "Run Date", 8) == 0) {
		for (size_t i = 0; i < n; ++i)
			if (line[i] >= '0' && line[i] <= '9')
				line[i] = '#';
	}
}

////////////////////////////////////////////////////////////////////////////////
// Compare the generated report against the committed expected output
static bool compare_files(czstring actual_path, czstring expected_path)
{
	FILE *fa = std::fopen(actual_path, "r");
	if (fa == NULL) {
		std::fprintf(stderr, "FAIL: cannot open generated report %s\n", actual_path);
		return false;
	}
	FILE *fe = std::fopen(expected_path, "r");
	if (fe == NULL) {
		// No baseline committed yet -- show the generated report so it can be
		// reviewed and checked in.
		std::fprintf(stderr, "NOTE: no expected output at %s; generated report was:\n",
			expected_path);
		char line[512];
		while (std::fgets(line, sizeof(line), fa))
			std::fputs(line, stderr);
		std::fclose(fa);
		return false;
	}

	bool equal = true;
	int  lineno = 0;
	char la[512], le[512];
	for (;;) {
		char *ra = std::fgets(la, sizeof(la), fa);
		char *re = std::fgets(le, sizeof(le), fe);
		++lineno;
		if (ra == NULL && re == NULL)
			break;
		if (ra == NULL || re == NULL) {
			std::fprintf(stderr, "FAIL: report length differs at line %d\n", lineno);
			equal = false;
			break;
		}
		normalize(la);
		normalize(le);

		if (std::strcmp(la, le) != 0) {
			std::fprintf(stderr, "FAIL: line %d differs\n  expected: [%s]\n  actual:   [%s]\n",
				lineno, le, la);
			equal = false;
		}
	}

	std::fclose(fa);
	std::fclose(fe);
	return equal;
}

////////////////////////////////////////////////////////////////////////////////
// Edit code / edit word rendering used by the report, checked directly.
// EDITWRD/EDITC return owning std::strings, so unlike RrecordPrint::edit()
// (which writes exactly strlen(edtWrd) bytes and does not terminate) they are
// safe to print and compare directly.
static void test_edit_rendering()
{
	// Edit word for a Zoned<9,2>: 9 digit positions (7 before the decimal
	// point, 2 after). Blanks and '0' are digit positions; ',' '.' 'CR' are not.
	// Layout: p,ppp,pp0.pp CR -- commas only print when a digit is to their left.
	czstring wrd = " ,   ,  0.  CR";

	std::string s = EDITWRD(Zoned<9, 2>(__D("1234.56")), wrd);
	std::fprintf(stderr, "EDITWRD(1234.56, \"%s\") = [%s]\n", wrd, s.c_str());
	CHECK(s.find("1,234.56") != std::string::npos);
	// Positive values leave the CR positions blank.
	CHECK(s.find("CR") == std::string::npos);

	s = EDITWRD(Zoned<9, 2>(__D("-320.75")), wrd);
	std::fprintf(stderr, "EDITWRD(-320.75, \"%s\") = [%s]\n", wrd, s.c_str());
	CHECK(s.find("320.75") != std::string::npos);
	CHECK(s.find("CR") != std::string::npos);

	s = EDITWRD(Zoned<9, 2>(__D("0.00")), wrd);
	std::fprintf(stderr, "EDITWRD(0.00,    \"%s\") = [%s]\n", wrd, s.c_str());
	// The '0' zero-suppression stop keeps the leading zero of "0.00".
	CHECK(s.find("0.00") != std::string::npos);

	// Edit codes used by the detail lines.
	s = EDITC(Zoned<9, 2>(__D("1234.56")), 'J');
	std::fprintf(stderr, "EDITC(1234.56, 'J') = [%s]\n", s.c_str());
	CHECK(s.find("1,234.56") != std::string::npos);

	s = EDITC(Zoned<9, 2>(__D("-320.75")), 'J');
	std::fprintf(stderr, "EDITC(-320.75, 'J') = [%s]\n", s.c_str());
	CHECK(s.find("320.75-") != std::string::npos);

	s = EDITC(Zoned<5, 0>(250), 'Z');
	std::fprintf(stderr, "EDITC(250, 'Z') = [%s]\n", s.c_str());
	CHECK(s.find("250") != std::string::npos);
	// 'Z' suppresses leading zeros entirely.
	CHECK(s.find("00250") == std::string::npos);
}

////////////////////////////////////////////////////////////////////////////////
// The negative detail row exercises signed construction and signed arithmetic
static void test_negative_row()
{
	Zoned<5, 0>  qty(-5);
	Zoned<9, 2>  amount(__D("-320.75"));
	Zoned<11, 2> extended = amount * qty;

	std::fprintf(stderr, "qty=%d amount=%s extended=%s\n",
		(int)qty,
		EDITC(amount, 'J').c_str(),
		EDITC(extended, 'J').c_str());

	CHECK((int)qty == -5);
	// negative * negative is positive
	CHECK(extended > 0);
	CHECK(EDITC(extended, 'J').find("1,603.75") != std::string::npos);
}

////////////////////////////////////////////////////////////////////////////////
// The floating currency symbol used on the grand total line.
//
// '$' occupies a digit position of its own, so the edit word carries one more
// position than the value has digits -- otherwise a full-width value uses every
// position and the symbol has nowhere to go (it silently disappears).
static void test_currency_edit_word()
{
	std::string s = EDITWRD(Zoned<11, 2>(__D("23427.97")), TOTAL_EDIT_WORD);
	std::fprintf(stderr, "currency mid  = [%s]\n", s.c_str());
	// Floats to sit immediately left of the leading digit, not far left.
	CHECK(s.find("$23,427.97") != std::string::npos);

	// Full width still shows the symbol, and all 9 integer digits.
	s = EDITWRD(Zoned<11, 2>(__D("999999999.99")), TOTAL_EDIT_WORD);
	std::fprintf(stderr, "currency wide = [%s]\n", s.c_str());
	CHECK(s.find("$999,999,999.99") != std::string::npos);

	// A credit balance keeps both the symbol and the CR.
	s = EDITWRD(Zoned<11, 2>(__D("-23427.97")), TOTAL_EDIT_WORD);
	std::fprintf(stderr, "currency neg  = [%s]\n", s.c_str());
	CHECK(s.find("$23,427.97") != std::string::npos);
	CHECK(s.find("CR") != std::string::npos);

	// Positive values leave the CR positions blank.
	s = EDITWRD(Zoned<11, 2>(__D("23427.97")), TOTAL_EDIT_WORD);
	CHECK(s.find("CR") == std::string::npos);

	// Exactly one symbol, whatever the value.
	CHECK(s.find('$') == s.rfind('$'));
}

int main()
{
	const char *actual   = "printer-output.txt";
	const char *expected = I2CLASS_TESTS_DIR "/printer-output.txt";

	generate_report(actual);
	test_edit_rendering();
	test_negative_row();
	test_currency_edit_word();

	if (!compare_files(actual, expected))
		++failures;

	if (failures == 0) {
		std::printf("All printer file tests passed.\n");
		return 0;
	}
	std::printf("%d printer file test(s) failed.\n", failures);
	return 1;
}
