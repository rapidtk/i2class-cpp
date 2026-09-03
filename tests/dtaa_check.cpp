// Regression tests for data area (Dtaara) support.
//
// On IBM i: Tests use real data areas via QXXRTVDA/QXXCHGDA APIs
// On Windows: QXXRTVDA/QXXCHGDA are stubs, so the tests exercise the
// Dtaara<> layout/assignment contract rather than persisted content.
//
// Tests verify:
// - Dtaara<T> adds no storage overhead beyond the wrapped type + name pointer
// - Assignment, comparison and blank-fill behave like the wrapped Fixed<>
// - in()/out() can be called without corrupting the buffer
// - Output is logged to dtaa-output.txt for inspection
#include <cstdio>
#include <cstring>

#include "RPGTypes.h"
#include "rglobal.h"

static int failures = 0;
static std::FILE *output_file = nullptr;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::fprintf(stderr, "FAIL: %s (dtaa_check.cpp:%d)\n", #cond, __LINE__); \
			if (output_file) std::fprintf(output_file, "FAIL: %s (line %d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

#define LOG(...) \
	do { \
		std::fprintf(stderr, __VA_ARGS__); \
		std::fprintf(stderr, "\n"); \
		if (output_file) { \
			std::fprintf(output_file, __VA_ARGS__); \
			std::fprintf(output_file, "\n"); \
		} \
	} while (0)

////////////////////////////////////////////////////////////////////////////////
// Test data area construction and blank initialization
static void test_dtaa_construction()
{
	LOG("=== Test: Data Area Construction ===");

	Dtaara<Fixed<20> > test_dtaa("TESTDTAA");

	// Dtaara<T> is T plus the name pointer it needs for QXXRTVDA/QXXCHGDA.
	CHECK(sizeof(test_dtaa) >= sizeof(Fixed<20>));
	LOG("sizeof(Dtaara<Fixed<20> >) = %d, sizeof(Fixed<20>) = %d",
		(int)sizeof(test_dtaa), (int)sizeof(Fixed<20>));

	// Fixed<> always default-constructs to blanks.
	CHECK(test_dtaa == BLANKS);
	LOG("Data area initialized to blanks: yes");

	CHECK(test_dtaa.len() == 20);
	LOG("Declared length: %d", test_dtaa.len());
}

////////////////////////////////////////////////////////////////////////////////
// Test assigning and reading back overlayed sub-fields of a data area
static void test_dtaa_field_overlay()
{
	LOG("=== Test: Data Area Field Overlay ===");

	Dtaara<Fixed<25> > data_area("FLDOVLAY");
	data_area = "HELLO     12345MORE TEXT";

	char buf[26];
	LOG("Data area contents: [%s]", data_area.c_str(buf, sizeof(buf)));

	// First 10 bytes are the text sub-field.
	Fixed<10> text_field = FixedTemp(data_area.overlay, 10);
	CHECK(text_field == "HELLO");
	char tbuf[11];
	LOG("Text sub-field (1-10): [%s]", text_field.c_str(tbuf, sizeof(tbuf)));

	// Next 5 bytes are a zoned numeric sub-field.
	Zoned<5, 0> &numeric_field = (Zoned<5, 0> &)data_area.overlay[10];
	CHECK((int)numeric_field == 12345);
	LOG("Numeric sub-field (11-15): %d", (int)numeric_field);

	CHECK(data_area.len() == 25);
}

////////////////////////////////////////////////////////////////////////////////
// Test data area read/write cycle.
//
// On Windows QXXRTVDA/QXXCHGDA are emulated by xxdtaa.cpp as flat
// "<name>.dtaara" files in the current directory, so the round trip really does
// persist. On IBM i the same calls hit the live system APIs.
static void test_dtaa_read_write()
{
	LOG("=== Test: Data Area Read/Write ===");

	// Start from a clean slate so the round trip is unambiguous.
	std::remove("COUNTER.dtaara");

	char buf[17];

	// Reading a data area that does not exist leaves the buffer untouched.
	Dtaara<Fixed<16> > missing_dtaa("COUNTER");
	missing_dtaa.in();
	CHECK(missing_dtaa == BLANKS);
	LOG("in() on absent data area leaves blanks: [%s]",
	    missing_dtaa.c_str(buf, sizeof(buf)));

	// Write a known value.
	Dtaara<Fixed<16> > counter_dtaa("COUNTER");
	counter_dtaa = "COUNTER:    0001";
	LOG("Before out(): [%s]", counter_dtaa.c_str(buf, sizeof(buf)));
	counter_dtaa.out();

	// out() must not disturb the in-memory copy.
	CHECK(counter_dtaa == "COUNTER:    0001");
	LOG("After out():  [%s]", counter_dtaa.c_str(buf, sizeof(buf)));

	// A separate handle on the same name must read that value back.
	Dtaara<Fixed<16> > read_dtaa("COUNTER");
	CHECK(read_dtaa == BLANKS);
	read_dtaa.in();
	LOG("After in():   [%s]", read_dtaa.c_str(buf, sizeof(buf)));
	CHECK(read_dtaa == "COUNTER:    0001");
	CHECK(read_dtaa == counter_dtaa);
	LOG("Round trip through the data area preserved the value: yes");

	// Update in place and re-read, the way an RPG counter data area is used.
	counter_dtaa = "COUNTER:    0002";
	counter_dtaa.out();
	read_dtaa.in();
	CHECK(read_dtaa == "COUNTER:    0002");
	LOG("After update: [%s]", read_dtaa.c_str(buf, sizeof(buf)));

	CHECK(read_dtaa.len() == 16);
}

////////////////////////////////////////////////////////////////////////////////
// Verify the persisted data area file matches the committed baseline
static void test_dtaa_persisted_bytes()
{
	LOG("=== Test: Persisted Data Area Bytes ===");

	Dtaara<Fixed<16> > counter_dtaa("COUNTER");
	counter_dtaa = "COUNTER:    0001";
	counter_dtaa.out();

	FILE *fp = std::fopen("COUNTER.dtaara", "rb");
	CHECK(fp != NULL);
	if (fp == NULL)
		return;

	char raw[64];
	size_t n = std::fread(raw, 1, sizeof(raw), fp);
	std::fclose(fp);

	LOG("COUNTER.dtaara holds %d bytes: [%.*s]", (int)n, (int)n, raw);
	CHECK(n == 16);
	CHECK(std::memcmp(raw, "COUNTER:    0001", 16) == 0);

	// Compare against the committed baseline, if present.
	FILE *fe = std::fopen(I2CLASS_TESTS_DIR "/COUNTER.dtaara", "rb");
	if (fe == NULL) {
		LOG("NOTE: no baseline at %s -- generated bytes shown above",
		    I2CLASS_TESTS_DIR "/COUNTER.dtaara");
		return;
	}
	char expected[64];
	size_t ne = std::fread(expected, 1, sizeof(expected), fe);
	std::fclose(fe);
	CHECK(ne == n);
	CHECK(ne == n && std::memcmp(raw, expected, n) == 0);
	LOG("Persisted bytes match the committed baseline: yes");
}

////////////////////////////////////////////////////////////////////////////////
// Test that two handles on the same data area name are independent buffers
static void test_dtaa_multiple_instances()
{
	LOG("=== Test: Multiple Data Area Instances ===");

	Dtaara<Fixed<30> > dtaa1("MULTITEST");
	Dtaara<Fixed<30> > dtaa2("MULTITEST");

	CHECK(dtaa1.len() == dtaa2.len());
	CHECK(dtaa1.len() == 30);
	LOG("Both handles declare length %d", dtaa1.len());

	dtaa1 = "Instance1 Content Here";

	char buf[31];
	LOG("dtaa1: [%s]", dtaa1.c_str(buf, sizeof(buf)));
	LOG("dtaa2: [%s]", dtaa2.c_str(buf, sizeof(buf)));

	// Separate storage: assigning to one must not disturb the other.
	CHECK(dtaa1 == "Instance1 Content Here");
	CHECK(dtaa2 == BLANKS);
	LOG("Handles hold independent buffers: yes");
}

////////////////////////////////////////////////////////////////////////////////
// Test a data area used as a structured configuration record
static void test_dtaa_in_record_structure()
{
	LOG("=== Test: Data Area as Config Record ===");

	struct ConfigArea {
		Fixed<10>   program_name;
		Fixed<4>    version;
		Zoned<5, 0> max_records;
	};

	CHECK(sizeof(ConfigArea) == 10 + 4 + 5);
	LOG("sizeof(ConfigArea) = %d (expected 19)", (int)sizeof(ConfigArea));

	Dtaara<ConfigArea> config_dtaa("CONFIG");
	config_dtaa.program_name = "PAYROLL";
	config_dtaa.version      = "V1R0";
	config_dtaa.max_records  = 500;

	char pbuf[11], vbuf[5];
	LOG("program_name = [%s]", config_dtaa.program_name.c_str(pbuf, sizeof(pbuf)));
	LOG("version      = [%s]", config_dtaa.version.c_str(vbuf, sizeof(vbuf)));
	LOG("max_records  = %d", (int)config_dtaa.max_records);

	CHECK(config_dtaa.program_name == "PAYROLL");
	CHECK(config_dtaa.version == "V1R0");
	CHECK((int)config_dtaa.max_records == 500);

	// Round-trip through the data area APIs must not disturb the struct.
	config_dtaa.out();
	CHECK(config_dtaa.program_name == "PAYROLL");
	LOG("Struct intact after out(): yes");
}

int main()
{
	output_file = std::fopen("dtaa-output.txt", "w");
	if (output_file) {
		std::fprintf(output_file, "Data Area (Dtaara) Regression Test Output\n");
		std::fprintf(output_file, "=========================================\n\n");
#if defined(__OS400__) || defined(__IBMCPP__)
		std::fprintf(output_file, "Platform: IBM i (QXXRTVDA/QXXCHGDA are live)\n\n");
#else
		std::fprintf(output_file, "Platform: non-IBM i (QXXRTVDA/QXXCHGDA are stubs)\n\n");
#endif
	}

	test_dtaa_construction();
	test_dtaa_field_overlay();
	test_dtaa_read_write();
	test_dtaa_persisted_bytes();
	test_dtaa_multiple_instances();
	test_dtaa_in_record_structure();

	if (failures == 0) {
		LOG("\n*** All data area tests PASSED ***");
		if (output_file) std::fclose(output_file);
		return 0;
	}
	LOG("\n*** %d data area test(s) FAILED ***", failures);
	if (output_file) std::fclose(output_file);
	return 1;
}
