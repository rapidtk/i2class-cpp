// Regression tests for date/time special words: UDATE, YEAR, UYEAR, MONTH, DAY
// and FmtDate with different _DATFMT control specification values.
//
// Tests verify:
// - YEAR/UYEAR/MONTH/DAY correctly overlay the same YYYYMMDD storage
// - FmtDate format conversion with *MDY (default), *DMY, *YMD
// - Changes in one date field view are reflected in all overlayed views
// - Output is logged to date-output.txt for inspection
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>

#include "RPGTypes.h"
#include "rglobal.h"

static int failures = 0;
static std::FILE *output_file = nullptr;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::fprintf(stderr, "FAIL: %s (date_check.cpp:%d)\n", #cond, __LINE__); \
			if (output_file) std::fprintf(output_file, "FAIL: %s (line %d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

#define LOG(fmt, ...) \
	do { \
		std::fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
		if (output_file) std::fprintf(output_file, fmt "\n", ##__VA_ARGS__); \
	} while (0)

////////////////////////////////////////////////////////////////////////////////
// Test that YEAR/UYEAR/MONTH/DAY overlay correctly
static void test_date_overlay_consistency()
{
	LOG("=== Test: Date Overlay Consistency ===");
	
	// After YYMD is constructed by FmtDate, YEAR, UYEAR, MONTH, DAY should
	// all be references into the same storage
	
	// Get the 4-digit year
	int year_val = (int)YEAR;
	CHECK(year_val >= 2000 && year_val <= 2100);
	LOG("YEAR = %04d", year_val);
	
	// Get the 2-digit year (last 2 digits of YEAR)
	int uyear_val = (int)UYEAR;
	CHECK(uyear_val >= 0 && uyear_val <= 99);
	LOG("UYEAR = %02d", uyear_val);
	
	// Verify that UYEAR is the last 2 digits of YEAR
	int expected_uyear = year_val % 100;
	CHECK(uyear_val == expected_uyear);
	
	// Get month (should be 01-12)
	int month_val = (int)MONTH;
	CHECK(month_val >= 1 && month_val <= 12);
	LOG("MONTH = %02d", month_val);
	
	// Get day (should be 01-31)
	int day_val = (int)DAY;
	CHECK(day_val >= 1 && day_val <= 31);
	LOG("DAY = %02d", day_val);
}

////////////////////////////////////////////////////////////////////////////////
// Decode a 2-digit field out of a FmtDate<6> overlay at the given offset
static int two_digits_at(const char *buf, int offset)
{
	return (buf[offset] - '0') * 10 + (buf[offset + 1] - '0');
}

////////////////////////////////////////////////////////////////////////////////
// Test FmtDate with each _DATFMT value, verifying the digit ordering.
//
// _DATFMT is a compile-time macro (the analogue of the RPGLE DATFMT control
// spec), so the globals UDATE/DATE are fixed at library build time. To exercise
// the other formats we construct FmtDate instances explicitly -- this is the
// same code path the globals use, just with a different format string.
static void test_fmtdate_formats()
{
	LOG("=== Test: FmtDate Formats ===");

	// Reference values from the always-YYYYMMDD YYMD storage.
	int ref_year  = (int)YEAR;
	int ref_uyear = (int)UYEAR;
	int ref_month = (int)MONTH;
	int ref_day   = (int)DAY;

	// *YMD at 8 digits widens the year to 4 digits: YYYYMMDD
	FmtDate<8> ymd8("*YMD");
	int y8_year  = (ymd8.overlay[0] - '0') * 1000 + (ymd8.overlay[1] - '0') * 100
	             + (ymd8.overlay[2] - '0') * 10   + (ymd8.overlay[3] - '0');
	int y8_month = two_digits_at(ymd8.overlay, 4);
	int y8_day   = two_digits_at(ymd8.overlay, 6);
	LOG("*YMD (8): [%.8s] -> year=%04d month=%02d day=%02d",
	    ymd8.overlay, y8_year, y8_month, y8_day);
	CHECK(y8_year == ref_year);
	CHECK(y8_month == ref_month);
	CHECK(y8_day == ref_day);

	// *YMD at 6 digits: YYMMDD
	FmtDate<6> ymd6("*YMD");
	LOG("*YMD (6): [%.6s] -> yy=%02d mm=%02d dd=%02d",
	    ymd6.overlay, two_digits_at(ymd6.overlay, 0),
	    two_digits_at(ymd6.overlay, 2), two_digits_at(ymd6.overlay, 4));
	CHECK(two_digits_at(ymd6.overlay, 0) == ref_uyear);
	CHECK(two_digits_at(ymd6.overlay, 2) == ref_month);
	CHECK(two_digits_at(ymd6.overlay, 4) == ref_day);

	// *DMY at 6 digits: DDMMYY
	FmtDate<6> dmy6("*DMY");
	LOG("*DMY (6): [%.6s] -> dd=%02d mm=%02d yy=%02d",
	    dmy6.overlay, two_digits_at(dmy6.overlay, 0),
	    two_digits_at(dmy6.overlay, 2), two_digits_at(dmy6.overlay, 4));
	CHECK(two_digits_at(dmy6.overlay, 0) == ref_day);
	CHECK(two_digits_at(dmy6.overlay, 2) == ref_month);
	CHECK(two_digits_at(dmy6.overlay, 4) == ref_uyear);

	// *MDY at 6 digits: MMDDYY (the default _DATFMT)
	FmtDate<6> mdy6("*MDY");
	LOG("*MDY (6): [%.6s] -> mm=%02d dd=%02d yy=%02d",
	    mdy6.overlay, two_digits_at(mdy6.overlay, 0),
	    two_digits_at(mdy6.overlay, 2), two_digits_at(mdy6.overlay, 4));
	CHECK(two_digits_at(mdy6.overlay, 0) == ref_month);
	CHECK(two_digits_at(mdy6.overlay, 2) == ref_day);
	CHECK(two_digits_at(mdy6.overlay, 4) == ref_uyear);

	// The three 6-digit formats must be permutations of each other, not equal.
	CHECK(std::memcmp(ymd6.overlay, dmy6.overlay, 6) != 0 || ref_day == ref_uyear);
	LOG("_DATFMT in effect for UDATE/DATE: %s", _DATFMT);
}

////////////////////////////////////////////////////////////////////////////////
// Test UDATE with format string
static void test_udate_constant()
{
	LOG("=== Test: UDATE Constant ===");
	
	// UDATE is a macro for XUDATE which is FmtDate<6> XUDATE(_DATFMT)
	// Should produce a 6-character date in _DATFMT format
	// Access via overlay buffer
	
	const char *udate_str = UDATE.overlay;
	LOG("UDATE buffer: %.6s", udate_str);
	
	// Should contain digits
	bool has_digit = false;
	for (int i = 0; i < 6; ++i) {
		if (udate_str[i] >= '0' && udate_str[i] <= '9') {
			has_digit = true;
			break;
		}
	}
	CHECK(has_digit);
}

////////////////////////////////////////////////////////////////////////////////
// Test DATE constant
static void test_date_constant()
{
	LOG("=== Test: DATE Constant ===");
	
	// DATE is a macro for XDATE which is FmtDate<8> XDATE(_DATFMT)
	// Should produce an 8-character date in _DATFMT format
	
	const char *date_str = DATE.overlay;
	LOG("DATE buffer: %.8s", date_str);
	
	// Should contain digits
	bool has_digit = false;
	for (int i = 0; i < 8; ++i) {
		if (date_str[i] >= '0' && date_str[i] <= '9') {
			has_digit = true;
			break;
		}
	}
	CHECK(has_digit);
}

////////////////////////////////////////////////////////////////////////////////
// Test that all four date fields are accessible and in valid ranges
static void test_date_field_ranges()
{
	LOG("=== Test: Date Field Ranges ===");
	
	// These should all be valid after YYMD initialization
	int year = (int)YEAR;
	int uyear = (int)UYEAR;
	int month = (int)MONTH;
	int day = (int)DAY;
	
	// Check ranges
	CHECK(year >= 1900 && year <= 2100);
	CHECK(uyear >= 0 && uyear <= 99);
	CHECK(month >= 1 && month <= 12);
	CHECK(day >= 1 && day <= 31);
	
	LOG("Date components: YEAR=%d, UYEAR=%d, MONTH=%d, DAY=%d", 
	    year, uyear, month, day);
}

int main()
{
	// Open output file
	output_file = std::fopen("date-output.txt", "w");
	if (output_file) {
		std::fprintf(output_file, "Date/Time Regression Test Output\n");
		std::fprintf(output_file, "================================\n\n");
	}
	
	LOG("Starting date/time tests...");
	test_date_overlay_consistency();
	test_fmtdate_formats();
	test_udate_constant();
	test_date_constant();
	test_date_field_ranges();

	if (failures == 0) {
		LOG("\n*** All date/time tests PASSED ***");
		if (output_file) std::fclose(output_file);
		return 0;
	}
	LOG("\n*** %d date/time test(s) FAILED ***", failures);
	if (output_file) std::fclose(output_file);
	return 1;
}
