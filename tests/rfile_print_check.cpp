// Regression tests for RrecordPrint::edit()/print() -- RPG-style printer edit-word and
// edit-code formatting. This logic had zero test coverage before (see rfile.cpp); writing
// these caught a real bug: the '0'/'*' zero-suppress marker position fell through to blank
// instead of showing '0' when the whole value was zero (edit word "  0.  " on 0.00 produced
// "   .00" instead of the correct "  0.00").
#include <cstdio>
#include <cstring>
#include <string>

#include "rfile.h"

static int failures = 0;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::fprintf(stderr, "FAIL: %s (rfile_print_check.cpp:%d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

class TestRecordPrint : public RrecordPrint
{
public:
	void flush() override {}
	using RrecordPrint::outputBuffer;
};

static void checkEditWord(Zoned<5,2> value, const char *edtWrd, const char *expected)
{
	char buf[64];
	std::memset(buf, '?', sizeof(buf));
	RrecordPrint::edit(buf, value, edtWrd);
	std::size_t len = std::strlen(edtWrd);
	buf[len] = '\0';
	if (std::strcmp(buf, expected) != 0)
		std::fprintf(stderr, "FAIL: edit(%.2f, \"%s\") -> \"%s\", expected \"%s\"\n",
			(double)value, edtWrd, buf, expected);
	CHECK(std::strcmp(buf, expected) == 0);
}

static void test_edit_word()
{
	checkEditWord(123.45, "  0.  ", "123.45");
	// Zero-suppress: leading non-significant zero blanked, the '0' marker itself always shows.
	checkEditWord(3.45, "  0.  ", "  3.45");
	// The '0' marker must show a digit even when the whole value is zero -- this was the bug.
	checkEditWord(0.00, "  0.  ", "  0.00");
	// No '-'/"CR" in the edit word -- RPG shows no sign at all, just the magnitude.
	checkEditWord(-3.45, "  0.  ", "  3.45");

	Zoned<5,2> value(123.45);
	CHECK(EDITWRD(value, "  0.  ") == "123.45");
	CHECK(value.editwrd("  0.  ") == "123.45");
	// 3 integer digits is a single group, so edit code 1 inserts no separator.
	CHECK(value.editc('1') == "123.45");
	Packed<5,2> packedValue(value);
	CHECK(packedValue.editwrd("  0.  ") == "123.45");
	CHECK(packedValue.editc('1') == "123.45");
	// 4 integer digits do span a separator position.
	Zoned<6,2> thousands(1234.56);
	CHECK(thousands.editc('1') == "1,234.56");
	// 6 integer digits: separator only between the two groups, not before the first.
	Zoned<8,2> sixDigits(123456.78);
	CHECK(sixDigits.editc('1') == "123,456.78");
}

// The worked examples from IBM's EDTCDE reference, which pin down where the
// digit-grouping separator does and does not belong.
// https://www.ibm.com/docs/en/i/7.4.0?topic=e-edtcde-display-files
static void test_edit_code_ibm_examples()
{
	// "PRICE 5 2 EDTCDE(J)" is documented as displaying 7 wide, as ddd.dd-
	// -- 3 integer digits are one group, so there is no separator.
	Zoned<5,2> price(123.45);
	CHECK(price.editc('J') == "123.45 ");
	Zoned<5,2> negPrice(-123.45);
	CHECK(negPrice.editc('J') == "123.45-");

	// "SALARY 8 2 EDTCDE(1)" is documented as displaying 10 wide, as ddd,ddd.dd
	// -- 6 integer digits are two groups with a single separator between them.
	Zoned<8,2> salary(123456.78);
	CHECK(salary.editc('1') == "123,456.78");

	// Edit code table: 1234567 with 2 decimals edits to 12,345.67 under code 1.
	Zoned<7,2> table1(12345.67);
	CHECK(table1.editc('1') == "12,345.67");

	// Same digits with no decimals edit to 1,234,567.
	Zoned<7,0> table2(1234567);
	CHECK(table2.editc('1') == "1,234,567");
}

static void checkEditCode(Zoned<7,2> value, int col, char edtCde, char fillChar, const char *expectedTail)
{
	TestRecordPrint r;
	std::memset(r.outputBuffer, '.', sizeof(r.outputBuffer) - 1);
	r.print(value, col, edtCde, fillChar);
	std::size_t tailLen = std::strlen(expectedTail);
	std::string actual(r.outputBuffer + col - static_cast<int>(tailLen) + 1, tailLen);
	if (actual != expectedTail)
		std::fprintf(stderr, "FAIL: print(%.2f, col=%d, '%c') -> \"%s\", expected \"%s\"\n",
			(double)value, col, edtCde, actual.c_str(), expectedTail);
	CHECK(actual == expectedTail);
}

static void test_edit_code()
{
	// '1': comma-inserted, no sign shown at all.
	checkEditCode(12345.67, 12, '1', ' ', "12,345.67");
	// 'J': comma-inserted, trailing '-' for negative.
	checkEditCode(-12345.67, 12, 'J', ' ', "12,345.67-");
	// 'A': comma-inserted, trailing "CR" for negative.
	checkEditCode(-12345.67, 12, 'A', ' ', "12,345.67CR");
	// 'X': no editing -- raw digits, no punctuation.
	checkEditCode(12345.67, 12, 'X', ' ', "1234567");
}

int main()
{
	test_edit_word();
	test_edit_code_ibm_examples();
	test_edit_code();

	if (failures == 0)
	{
		std::printf("All RrecordPrint edit/print tests passed.\n");
		return 0;
	}
	std::printf("%d RrecordPrint edit/print test(s) failed.\n", failures);
	return 1;
}
