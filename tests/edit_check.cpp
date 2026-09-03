// Edit code / edit word reference report.
//
// Renders the IBM i edit codes and a range of edit words against fixed sample
// values, through the same RfileLPT printer file the other tests use (a local
// text file on Windows, spooled output on IBM i). The generated report is
// diffed against the committed tests/edit-output.txt so any change in editing
// behaviour shows up as a reviewable diff.
//
// References (local copy of the tables: legacy-uef/EDTCDE-NOTES.md):
//   EDTCDE https://www.ibm.com/docs/en/i/7.4.0?topic=e-edtcde-display-files
//   EDTWRD https://www.ibm.com/docs/en/i/7.4.0?topic=e-edtwrd-display-files
//
// The committed report is what this implementation produces. Everything the
// references state outright is additionally asserted in check_documented()
// below, so the report cannot drift away from the documented behaviour without
// failing; the rest is review-by-eye.
//
// DEVIATIONS AND OPEN QUESTIONS -- see the report footer, which prints these
// so they travel with the artifact rather than only living in this comment.
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
			std::fprintf(stderr, "FAIL: %s (edit_check.cpp:%d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

// Every IBM i edit code, in the order the reference tables list them.
static const char ALL_CODES[] = "1234ABCDJKLMNOPQWYZX";
// Only these accept an asterisk fill or floating currency symbol.
static const char FILLABLE_CODES[] = "1234ABCDJKLMNOPQ";

////////////////////////////////////////////////////////////////////////////////
// The six sample values the EDTCDE output table is built around, in its order.
enum SampleId {
	S_7_2 = 0,   // 1234567 with 2 decimals -> 12345.67
	S_7_0,       // 1234567 with 0 decimals -> 1234567
	S_7_3_NEG,   // fraction only, negative -> -.125
	S_3_0_NEG,   // small negative          -> -125
	S_6_2_ZERO,  // zero balance, 2 decimals
	S_6_0_ZERO,  // zero balance, no decimals
	SAMPLE_COUNT
};

static czstring SAMPLE_HEADING[SAMPLE_COUNT] = {
	"12345.67", "1234567", "-.125", "-125", "0.00", "0"
};

static std::string edited(SampleId id, char code, char fill = ' ')
{
	switch (id) {
	case S_7_2:      return EDITC(Zoned<7,2>(__D("12345.67")), code, fill);
	case S_7_0:      return EDITC(Zoned<7,0>(1234567), code, fill);
	case S_7_3_NEG:  return EDITC(Zoned<7,3>(__D("-0.125")), code, fill);
	case S_3_0_NEG:  return EDITC(Zoned<3,0>(-125), code, fill);
	case S_6_2_ZERO: return EDITC(Zoned<6,2>(__D("0.00")), code, fill);
	case S_6_0_ZERO: return EDITC(Zoned<6,0>(0), code, fill);
	default:         return std::string();
	}
}

// Column layout for the edit code grids.
enum {
	COL_LABEL = 1,
	COL_FIRST = 8,
	COL_WIDTH = 17
};

////////////////////////////////////////////////////////////////////////////////
static void code_grid(RrecordLPT &rcd, czstring title, czstring codes, char fill)
{
	rcd.printl(title, COL_LABEL);
	rcd.space(2);

	rcd.printl("Code", COL_LABEL);
	for (int s = 0; s < SAMPLE_COUNT; ++s)
		rcd.printl(SAMPLE_HEADING[s], COL_FIRST + s * COL_WIDTH);
	rcd.space(1);

	rcd.printl("----", COL_LABEL);
	for (int s = 0; s < SAMPLE_COUNT; ++s)
		rcd.printl("---------------", COL_FIRST + s * COL_WIDTH);
	rcd.space(1);

	for (const char *c = codes; *c; ++c) {
		char label[2] = { *c, '\0' };
		rcd.printl(label, COL_LABEL);
		for (int s = 0; s < SAMPLE_COUNT; ++s) {
			// Bracketed so leading and trailing blanks stay visible.
			std::string cell = "[" + edited((SampleId)s, *c, fill) + "]";
			rcd.printl(cell.c_str(), COL_FIRST + s * COL_WIDTH);
		}
		rcd.space(1);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Digit grouping across every integer width from 1 to 9. A separator belongs
// between groups of three and never before the leading group, so the widths
// that are exact multiples of 3 are the interesting ones.
static std::string grouped(int intDigits)
{
	switch (intDigits) {
	case 1: return EDITC(Zoned<1,0>(5), '1');
	case 2: return EDITC(Zoned<2,0>(45), '1');
	case 3: return EDITC(Zoned<3,0>(345), '1');
	case 4: return EDITC(Zoned<4,0>(2345), '1');
	case 5: return EDITC(Zoned<5,0>(12345), '1');
	case 6: return EDITC(Zoned<6,0>(123456), '1');
	case 7: return EDITC(Zoned<7,0>(1234567), '1');
	case 8: return EDITC(Zoned<8,0>(12345678), '1');
	case 9: return EDITC(Zoned<9,0>(123456789), '1');
	default: return std::string();
	}
}

static czstring GROUPED_EXPECTED[10] = {
	"", "5", "45", "345", "2,345", "12,345",
	"123,456", "1,234,567", "12,345,678", "123,456,789"
};

static void grouping_section(RrecordLPT &rcd)
{
	rcd.printl("Digit grouping, edit code 1, integer widths 1-9", COL_LABEL);
	rcd.space(2);

	rcd.printl("Digits", COL_LABEL);
	rcd.printl("Displayed as", 12);
	rcd.printl("Groups of 3", 32);
	rcd.space(1);
	rcd.printl("------", COL_LABEL);
	rcd.printl("----------------", 12);
	rcd.printl("-----------", 32);
	rcd.space(1);

	for (int d = 1; d <= 9; ++d) {
		char label[4];
		std::sprintf(label, "%d", d);
		std::string cell = "[" + grouped(d) + "]";
		rcd.printl(label, COL_LABEL);
		rcd.printl(cell.c_str(), 12);
		rcd.printl(d % 3 == 0 ? "exact" : "partial", 32);
		rcd.space(1);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Edit words. A 7,2 field has 5 integer digit positions and 2 fractional ones.
// Blanks and the zero-suppression stop ('0' or '*') are digit positions and
// must total the field length; ',' '.' '&' 'CR' '-' and constants are not.
struct WordSample {
	czstring    word;
	int         digits;
	int         precision;
	const char *value;
	czstring    note;
};

static const WordSample WORD_SAMPLES[] = {
	// -- Body: blanks and the zero-suppression stop --------------------------
	{ "       ",         7, 0, "1234567",   "all blanks, no stop" },
	{ "       ",         7, 0, "35",        "all blanks, leading zeros gone" },
	{ "      0",         7, 0, "35",        "stop at units" },
	{ "      0",         7, 0, "0",         "stop shows a zero balance" },
	{ "     0 ",         7, 0, "0",         "stop one left of units" },

	// -- Commas and the decimal point ----------------------------------------
	{ "  ,  0.  ",       7, 2, "12345.67",  "commas and decimal point" },
	{ "  ,  0.  ",       7, 2, "0.07",      "separators left of digits blanked" },
	{ "  ,  0.  ",       7, 2, "0.00",      "zero balance" },

	// -- Status: CR and minus print only when negative ------------------------
	{ "  ,  0.  CR",     7, 2, "-12345.67", "CR status, negative" },
	{ "  ,  0.  CR",     7, 2, "12345.67",  "CR status, positive" },
	{ "  ,  0.  -",      7, 2, "-12345.67", "minus status, negative" },
	{ "  ,  0.  -",      7, 2, "12345.67",  "minus status, positive" },
	{ "       -",        7, 0, "-35",       "minus, no stop character" },

	// -- Asterisk protection --------------------------------------------------
	{ "  ,  *.  ",       7, 2, "1357.92",   "asterisk fill" },
	{ "  ,  *.  ",       7, 2, "0.00",      "asterisk fill, zero balance" },
	{ "      *",         7, 0, "35",        "asterisk fill, no decimals" },

	// -- Currency symbol ------------------------------------------------------
	{ "    $0.  ",       7, 2, "12345.67",  "floating $, reference example" },
	{ "    $0.  ",       7, 2, "0.05",      "floating $, follows the digits" },
	{ "    $0.  ",       7, 2, "0.00",      "floating $, zero balance" },
	{ "$    0.  ",       7, 2, "12345.67",  "fixed $, leftmost position" },
	{ "$    0.  ",       7, 2, "0.05",      "fixed $ stays put" },
	{ "$    0.  ",       7, 2, "0.00",      "fixed $, zero balance" },
	{ "  ,  $0.  ",      7, 2, "12345.67",  "floating $, no room -- see (h)" },
	{ "  ,  $0.  ",      7, 2, "0.05",      "floating $ with commas, short value" },

	// -- Ampersand puts a blank in the output and is not a digit position -----
	{ "   &  &  ",       7, 0, "1234567",   "ampersand blanks" },
	{ "  0&  ",          5, 0, "12345",     "ampersand mid-body" },

	// -- Expansion: constants right of the status are always written ----------
	{ "  ,  0.  &NET",   7, 2, "12345.67",  "trailing constant" },
	{ "  ,  0.  CR&NET", 7, 2, "-12345.67", "CR then constant" },
	{ "  ,  0.  CR&NET", 7, 2, "12345.67",  "constant prints, CR does not" },
	{ "    0&DOLLARS",   5, 0, "12345",     "word constant" },

	// -- Constants left of the significant digits are blanked -----------------
	{ "  0/  /  ",       7, 0, "1230576",   "date-style constants" },
	{ "0 /  /  ",        6, 0, "93076",     "leading constant after a stop" },
};
static const int WORD_COUNT = (int)(sizeof(WORD_SAMPLES) / sizeof(WORD_SAMPLES[0]));

static std::string edited_word(const WordSample &s)
{
	if (s.digits == 7 && s.precision == 2)
		return EDITWRD(Zoned<7,2>(__D(s.value)), s.word);
	if (s.digits == 7 && s.precision == 0)
		return EDITWRD(Zoned<7,0>(__D(s.value)), s.word);
	if (s.digits == 6 && s.precision == 0)
		return EDITWRD(Zoned<6,0>(__D(s.value)), s.word);
	if (s.digits == 5 && s.precision == 0)
		return EDITWRD(Zoned<5,0>(__D(s.value)), s.word);
	return std::string();
}

static void word_section(RrecordLPT &rcd)
{
	rcd.printl("Edit words (EDTWRD)", COL_LABEL);
	rcd.space(2);

	rcd.printl("Edit word", COL_LABEL);
	rcd.printl("Field", 20);
	rcd.printl("Value", 28);
	rcd.printl("Displayed as", 41);
	rcd.printl("Note", 62);
	rcd.space(1);

	rcd.printl("-----------------", COL_LABEL);
	rcd.printl("-----", 20);
	rcd.printl("----------", 28);
	rcd.printl("------------------", 41);
	rcd.printl("---------------------------------", 62);
	rcd.space(1);

	for (int i = 0; i < WORD_COUNT; ++i) {
		const WordSample &s = WORD_SAMPLES[i];

		std::string word = "[";
		word += s.word;
		word += "]";

		char shape[16];
		std::sprintf(shape, "%d,%d", s.digits, s.precision);

		std::string out = "[" + edited_word(s) + "]";

		rcd.printl(word.c_str(), COL_LABEL);
		rcd.printl(shape, 20);
		rcd.printl(s.value, 28);
		rcd.printl(out.c_str(), 41);
		rcd.printl(s.note, 62);
		rcd.space(1);
	}
}

////////////////////////////////////////////////////////////////////////////////
// The three DDS field definitions worked through in the EDTCDE reference.
static void dds_section(RrecordLPT &rcd)
{
	rcd.printl("DDS examples from the EDTCDE reference", COL_LABEL);
	rcd.space(2);

	rcd.printl("Field", COL_LABEL);
	rcd.printl("Definition", 12);
	rcd.printl("Documented", 36);
	rcd.printl("Width", 50);
	rcd.printl("Displayed as", 58);
	rcd.space(1);
	rcd.printl("------", COL_LABEL);
	rcd.printl("----------------------", 12);
	rcd.printl("-----------", 36);
	rcd.printl("-----", 50);
	rcd.printl("------------------", 58);
	rcd.space(1);

	std::string price = EDITC(Zoned<5,2>(__D("-123.45")), 'J');
	rcd.printl("PRICE", COL_LABEL);
	rcd.printl("5  2   EDTCDE(J)", 12);
	rcd.printl("ddd.dd-", 36);
	char w[8];
	std::sprintf(w, "%d", (int)price.size());
	rcd.printl(w, 50);
	rcd.printl(("[" + price + "]").c_str(), 58);
	rcd.space(1);

	std::string sales = EDITC(Zoned<7,2>(__D("-12345.67")), 'K', '$');
	rcd.printl("SALES", COL_LABEL);
	rcd.printl("7  2   EDTCDE(K $)", 12);
	rcd.printl("$dd,ddd.dd-", 36);
	std::sprintf(w, "%d", (int)sales.size());
	rcd.printl(w, 50);
	rcd.printl(("[" + sales + "]").c_str(), 58);
	rcd.space(1);

	std::string salary = EDITC(Zoned<8,2>(__D("123456.78")), '1', '*');
	rcd.printl("SALARY", COL_LABEL);
	rcd.printl("8  2   EDTCDE(1 *)", 12);
	rcd.printl("ddd,ddd.dd", 36);
	std::sprintf(w, "%d", (int)salary.size());
	rcd.printl(w, 50);
	rcd.printl(("[" + salary + "]").c_str(), 58);
	rcd.space(1);
}

////////////////////////////////////////////////////////////////////////////////
static czstring FOOTER[] = {
	"Deviations from the reference, and open questions:",
	"",
	"FIXED, having been caught by this report:",
	"",
	"  a. A '$' in the leftmost position of an edit word is a fixed currency",
	"     symbol and prints in place. It previously vanished entirely.",
	"",
	"  b. EDTCDE(c $) emitted the symbol next to the zero-suppression stop,",
	"     mid-number, where it displaced the digit grouping: the reference's",
	"     own SALES example rendered as $1,2345.67- instead of $12,345.67-.",
	"     The symbol now reserves the leftmost position and is not a digit",
	"     position, so it cannot shift the commas.",
	"",
	"  c. Edit codes N-Q put the minus in front, and the reference shows it",
	"     against the first significant digit (-.125). It was pinned to the",
	"     far left instead (-     .125), so it now floats like the currency",
	"     symbol, falling back to the coded position when the digits fill the",
	"     field and there is nowhere to float to.",
	"",
	"  d. Asterisk protection left the zero-suppression stop showing a '0', so",
	"     a zero balance edited to ******0 rather than a full field of",
	"     asterisks as the reference requires.",
	"",
	"  e. Codes 1/3/A/C/J/L/N/P showed blanks instead of 0 for a zero balance",
	"     in a field with no decimals.",
	"",
	"WHERE THE REFERENCE LOOKS WRONG:",
	"",
	"  f. EDTWRD gives the floating currency example as EDTWRD('    $0. '),",
	"     which is only 6 digit positions for a 7 digit field. The trailing",
	"     blank is lost in the text rendering; it has to be '    $0.  '.",
	"     This report uses the corrected form.",
	"",
	"OPEN QUESTIONS, worth confirming against a real system:",
	"",
	"  g. EDTCDE(c $) is called a floating symbol, but the only worked example",
	"     is full width, where floating and fixed are indistinguishable. We",
	"     reserve the leftmost position, which reproduces that example exactly;",
	"     a short value therefore renders $     0.05 rather than      $0.05.",
	"     Which is correct is not documented.",
	"",
	"  h. A hand-written edit word that codes the symbol next to the stop, as",
	"     the EDTWRD rules describe, has no spare position for it once the",
	"     value fills every digit position -- '  ,  $0.  ' on 12345.67 has to",
	"     put the symbol somewhere and there is no room to the left. The",
	"     reference's own rule that at least one leading zero is suppressed",
	"     suggests such a word is simply not meant to be used at full width.",
	"",
	"  i. A floating minus and a floating currency symbol would want the same",
	"     position. EDTCDE(N $) therefore leaves the minus at the far left.",
	"     The reference does not show this combination.",
	"",
	"  k. With asterisk protection and a floating minus, the minus leaves its",
	"     coded position blank rather than filled -- see the N and P rows of the",
	"     asterisk grid, which read ' ****-.125'. Whether that leading position",
	"     should be an asterisk is not documented.",
	"",
	"  j. Edit codes W and Y build their date pattern from the digit count",
	"     alone, so a field whose width is not a date shape gives odd output",
	"     (see the W and Y rows). The reference does not say what should",
	"     happen there.",
	NULL
};

static void footer_section(RrecordLPT &rcd)
{
	for (int i = 0; FOOTER[i]; ++i) {
		if (FOOTER[i][0])
			rcd.printl(FOOTER[i], COL_LABEL);
		rcd.space(1);
	}
}

////////////////////////////////////////////////////////////////////////////////
static void generate_report(czstring path)
{
	AS400 as400;
	RfileLPT prt(as400, path, MAX_PRINT_FILE_WIDTH);
	prt.open("W");

	RrecordLPT rcd;
	prt.setRecordFormat(rcd);

	rcd.printl("Edit Code / Edit Word Reference", 25);
	rcd.space(2);

	code_grid(rcd, "Edit codes (EDTCDE), no fill character", ALL_CODES, ' ');
	rcd.space(2);
	code_grid(rcd, "Edit codes with asterisk fill, EDTCDE(c *)", FILLABLE_CODES, '*');
	rcd.space(2);
	code_grid(rcd, "Edit codes with floating currency, EDTCDE(c $)", FILLABLE_CODES, '$');
	rcd.space(2);
	grouping_section(rcd);
	rcd.space(2);
	dds_section(rcd);
	rcd.space(2);
	word_section(rcd);
	rcd.space(2);
	footer_section(rcd);

	rcd.printl("END OF REPORT", COL_LABEL);
	rcd.space(1);

	prt.close();
}

////////////////////////////////////////////////////////////////////////////////
// Results the references state outright. Trailing and leading blanks are
// ignored so these compare the significant text only.
static std::string trim(const std::string &s)
{
	size_t b = s.find_first_not_of(' ');
	if (b == std::string::npos)
		return std::string();
	size_t e = s.find_last_not_of(' ');
	return s.substr(b, e - b + 1);
}

static void expect(czstring what, const std::string &actual, czstring documented)
{
	if (trim(actual) != documented) {
		std::fprintf(stderr, "FAIL: %s -> [%s], reference documents [%s]\n",
			what, trim(actual).c_str(), documented);
		++failures;
	}
}

static void expect_code(SampleId id, char code, czstring documented)
{
	char what[64];
	std::sprintf(what, "code '%c' on %s", code, SAMPLE_HEADING[id]);
	expect(what, edited(id, code), documented);
}

static void check_documented()
{
	// -- EDTCDE output table, row by row -------------------------------------
	// Codes 1 and 2 insert commas; 3 and 4 do not.
	expect_code(S_7_2, '1', "12,345.67");
	expect_code(S_7_0, '1', "1,234,567");
	expect_code(S_7_2, '2', "12,345.67");
	expect_code(S_7_0, '2', "1,234,567");
	expect_code(S_7_2, '3', "12345.67");
	expect_code(S_7_0, '3', "1234567");
	expect_code(S_7_2, '4', "12345.67");
	expect_code(S_7_0, '4', "1234567");

	// The fraction-only and small-negative columns.
	expect_code(S_7_3_NEG, '1', ".125");
	expect_code(S_7_3_NEG, '2', ".125");
	expect_code(S_3_0_NEG, '1', "125");
	expect_code(S_3_0_NEG, '2', "125");

	// A-D append CR when negative; J-M append a trailing minus.
	expect_code(S_7_3_NEG, 'A', ".125CR");
	expect_code(S_7_3_NEG, 'B', ".125CR");
	expect_code(S_7_3_NEG, 'C', ".125CR");
	expect_code(S_7_3_NEG, 'D', ".125CR");
	expect_code(S_3_0_NEG, 'A', "125CR");
	expect_code(S_3_0_NEG, 'C', "125CR");
	expect_code(S_7_3_NEG, 'J', ".125-");
	expect_code(S_7_3_NEG, 'L', ".125-");
	expect_code(S_3_0_NEG, 'J', "125-");
	expect_code(S_3_0_NEG, 'L', "125-");

	// N-Q put the minus in front.
	expect_code(S_7_3_NEG, 'N', "-.125");
	expect_code(S_7_3_NEG, 'P', "-.125");
	expect_code(S_3_0_NEG, 'N', "-125");
	expect_code(S_3_0_NEG, 'P', "-125");
	expect_code(S_7_2, 'N', "12,345.67");
	expect_code(S_7_2, 'P', "12345.67");

	// Z removes the sign and suppresses leading zeros.
	expect_code(S_7_0, 'Z', "1234567");
	expect_code(S_3_0_NEG, 'Z', "125");
	expect_code(S_6_0_ZERO, 'Z', "");

	// Zero balance: 1/3/A/C/J/L/N/P print it, 2/4/B/D/K/M/O/Q blank it.
	expect_code(S_6_2_ZERO, '1', ".00");
	expect_code(S_6_0_ZERO, '1', "0");
	expect_code(S_6_2_ZERO, '3', ".00");
	expect_code(S_6_0_ZERO, '3', "0");
	expect_code(S_6_0_ZERO, 'A', "0");
	expect_code(S_6_0_ZERO, 'C', "0");
	expect_code(S_6_0_ZERO, 'J', "0");
	expect_code(S_6_0_ZERO, 'L', "0");
	expect_code(S_6_0_ZERO, 'N', "0");
	expect_code(S_6_0_ZERO, 'P', "0");
	expect_code(S_6_2_ZERO, '2', "");
	expect_code(S_6_0_ZERO, '2', "");
	expect_code(S_6_2_ZERO, '4', "");
	expect_code(S_6_0_ZERO, '4', "");
	expect_code(S_6_0_ZERO, 'B', "");
	expect_code(S_6_0_ZERO, 'D', "");
	expect_code(S_6_0_ZERO, 'K', "");
	expect_code(S_6_0_ZERO, 'M', "");
	expect_code(S_6_0_ZERO, 'O', "");
	expect_code(S_6_0_ZERO, 'Q', "");

	// -- Digit grouping -------------------------------------------------------
	for (int d = 1; d <= 9; ++d) {
		char what[48];
		std::sprintf(what, "edit code 1 on %d integer digits", d);
		expect(what, grouped(d), GROUPED_EXPECTED[d]);
	}

	// -- DDS examples ---------------------------------------------------------
	// "The display length for PRICE is 7 ... edited as ddd.dd-"
	std::string price = EDITC(Zoned<5,2>(__D("-123.45")), 'J');
	CHECK(price.size() == 7);
	expect("PRICE 5,2 EDTCDE(J)", price, "123.45-");

	// "The display length for SALES is 11 ... edited as $dd,ddd.dd-"
	std::string sales = EDITC(Zoned<7,2>(__D("-12345.67")), 'K', '$');
	CHECK(sales.size() == 11);
	expect("SALES 7,2 EDTCDE(K $)", sales, "$12,345.67-");

	// "The display length for SALARY is 10 ... edited as ddd,ddd.dd"
	std::string salary = EDITC(Zoned<8,2>(__D("123456.78")), '1', '*');
	CHECK(salary.size() == 10);
	expect("SALARY 8,2 EDTCDE(1 *)", salary, "123,456.78");

	// -- Asterisk fill --------------------------------------------------------
	// "An asterisk is printed for each zero that is suppressed. A complete
	// field of asterisks is printed for a zero balance field."
	std::string starZero = EDITC(Zoned<6,0>(0), '1', '*');
	CHECK(starZero.find_first_not_of('*') == std::string::npos);
	std::string starPart = EDITC(Zoned<6,0>(135), '1', '*');
	// Four asterisks, not three: the comma position sits left of the significant
	// digits, and the reference says separators there are replaced by an asterisk
	// when asterisk protection is in use.
	expect("asterisk fill on 135", starPart, "****135");

	// -- Fixed currency symbol ------------------------------------------------
	// "If you code a currency symbol in the farthest left position of the edit
	// word, it is fixed and prints in the same location each time."
	expect("fixed $, 12345.67", EDITWRD(Zoned<7,2>(__D("12345.67")), "$    0.  "),
		"$12345.67");
	expect("fixed $, 0.05", EDITWRD(Zoned<7,2>(__D("0.05")), "$    0.  "),
		"$    0.05");
	expect("fixed $, 0.00", EDITWRD(Zoned<7,2>(__D("0.00")), "$    0.  "),
		"$    0.00");

	// -- Floating currency symbol ---------------------------------------------
	// The reference's own worked example, with the missing blank restored.
	expect("floating $, 12345.67", EDITWRD(Zoned<7,2>(__D("12345.67")), "    $0.  "),
		"$12345.67");
	expect("floating $, 0.05", EDITWRD(Zoned<7,2>(__D("0.05")), "    $0.  "),
		"$0.05");

	// Exactly one symbol, wherever it lands.
	std::string f = EDITWRD(Zoned<7,2>(__D("0.05")), "    $0.  ");
	CHECK(f.find('$') == f.rfind('$'));

	// -- Status positions -----------------------------------------------------
	// "If the sign in the edited output field is plus, these positions are
	// blanked out."
	CHECK(EDITWRD(Zoned<7,2>(__D("12345.67")), "  ,  0.  CR").find("CR")
		== std::string::npos);
	CHECK(EDITWRD(Zoned<7,2>(__D("-12345.67")), "  ,  0.  CR").find("CR")
		!= std::string::npos);

	// -- Expansion ------------------------------------------------------------
	// "The characters in the expansion portion of an edit word are always
	// written" -- the constant shows whether or not the value is negative.
	CHECK(EDITWRD(Zoned<7,2>(__D("12345.67")), "  ,  0.  CR&NET").find("NET")
		!= std::string::npos);
	CHECK(EDITWRD(Zoned<7,2>(__D("-12345.67")), "  ,  0.  CR&NET").find("NET")
		!= std::string::npos);

	// -- Ampersand ------------------------------------------------------------
	// "An ampersand causes a blank in the edited field. The ampersand is not
	// displayed."
	CHECK(EDITWRD(Zoned<7,0>(__D("1234567")), "   &  &  ").find('&')
		== std::string::npos);
}

////////////////////////////////////////////////////////////////////////////////
static bool compare_files(czstring actual_path, czstring expected_path)
{
	FILE *fa = std::fopen(actual_path, "r");
	if (fa == NULL) {
		std::fprintf(stderr, "FAIL: cannot open generated report %s\n", actual_path);
		return false;
	}
	FILE *fe = std::fopen(expected_path, "r");
	if (fe == NULL) {
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
		size_t na = std::strlen(la);
		while (na && (la[na - 1] == '\n' || la[na - 1] == '\r' || la[na - 1] == ' '))
			la[--na] = '\0';
		size_t ne = std::strlen(le);
		while (ne && (le[ne - 1] == '\n' || le[ne - 1] == '\r' || le[ne - 1] == ' '))
			le[--ne] = '\0';

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

int main()
{
	const char *actual   = "edit-output.txt";
	const char *expected = I2CLASS_TESTS_DIR "/edit-output.txt";

	generate_report(actual);
	check_documented();

	if (!compare_files(actual, expected))
		++failures;

	if (failures == 0) {
		std::printf("All edit code/word checks passed.\n");
		return 0;
	}
	std::printf("%d edit code/word check(s) failed.\n", failures);
	return 1;
}
