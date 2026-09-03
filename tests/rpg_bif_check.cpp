// Coverage for the RPG opcodes and built-in functions that had no tests: the array BIFs
// (XFOOT/SORTA/LOOKUP/ELEM), the string opcodes on Fixed<> (SCAN/CHECK/CHECKR/CAT/MOVEA),
// TESTN/TESTZ, the free MOVE/MOVEL overloads, and the numeric increment/modulo operators.
//
// Several of these were broken on arrival -- Fixed<> members are templates, so anything
// nothing called was never fully compiled. See the workspace docs for the details.
#include <cstdio>
#include <cstring>
#include <cmath>

#include "RPGTypes.h"

static int failures = 0;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::fprintf(stderr, "FAIL: %s (rpg_bif_check.cpp:%d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

////////////////////////////////////////////////////////////////////////////////
// %SCAN -- scan(char), scan(const char*), scan(FixedTemp)
static void test_scan()
{
	Fixed<10> f = "hello worl";

	CHECK(f.scan('h') == 1);
	CHECK(f.scan('w') == 7);
	CHECK(f.scan('l') == 3);   // first 'l', not a later one
	CHECK(f.scan('z') == 0);   // absent

	CHECK(f.scan("hello") == 1);
	CHECK(f.scan("wor") == 7);
	CHECK(f.scan("lo w") == 4);
	CHECK(f.scan("nope") == 0);
	CHECK(f.scan("hello worl") == 1);      // needle exactly fills the field
	CHECK(f.scan("hello world") == 0);     // needle longer than the field

	FixedTemp t("wor");
	CHECK(f.scan(t) == 7);
}

////////////////////////////////////////////////////////////////////////////////
// %CHECK / %CHECKR -- first/last character not in the given set
static void test_check()
{
	Fixed<6> f = "abcxbc";

	// 'x' at position 4 is the first character not in "abc"
	CHECK(f.check("abc") == 4);
	CHECK(f.check("abcx") == 0);       // every character is in the set
	CHECK(f.check("abc", 5) == 0);     // starting past the 'x'
	CHECK(f.check("bc") == 1);         // 'a' is the first not in "bc"

	// checkr walks backwards: last character not in the set
	CHECK(f.checkr("abc") == 4);
	CHECK(f.checkr("abcx") == 0);
	CHECK(f.checkr("bc") == 4);        // 'x' is the last not in "bc"

	Fixed<5> padded = "ab";            // Fixed<> blank-pads
	CHECK(padded.check(' ') == 1);     // first non-blank
	CHECK(padded.checkr(' ') == 2);    // last non-blank
}

////////////////////////////////////////////////////////////////////////////////
// CAT, and c_str()
static void test_cat_and_cstr()
{
	Fixed<12> c;
	c.cat(FixedTemp("ab"), FixedTemp("cd"));
	CHECK(std::strcmp(c.c_str(), "abcd        ") == 0);   // tail must be blanks

	Fixed<12> withBlanks;
	withBlanks.cat(FixedTemp("ab"), FixedTemp("cd"), 3);
	CHECK(std::strcmp(withBlanks.c_str(), "ab   cd     ") == 0);

	// c_str() shares one static buffer per Fixed<sz> instantiation -- by design (see
	// README known limitations), so only one result is live at a time. Use each result
	// before calling c_str() again on a same-size Fixed<>.
	Fixed<5> a = "AAAAA";
	Fixed<5> b = "BBBBB";
	CHECK(std::strcmp(a.c_str(), "AAAAA") == 0);
	CHECK(std::strcmp(b.c_str(), "BBBBB") == 0);

	// Caller-supplied buffer overload: no shared storage, no aliasing, truncates like
	// strlcpy() -- this is how to hold two same-size results live at once.
	char bufA[6], bufB[6];
	CHECK(a.c_str(bufA, sizeof(bufA)) == bufA);
	CHECK(b.c_str(bufB, sizeof(bufB)) == bufB);
	CHECK(std::strcmp(bufA, "AAAAA") == 0);
	CHECK(std::strcmp(bufB, "BBBBB") == 0);

	char small[3];
	a.c_str(small, sizeof(small));
	CHECK(std::strcmp(small, "AA") == 0);
}

////////////////////////////////////////////////////////////////////////////////
// MOVE / MOVEL / MOVEA / MOVEALL on Fixed<>
static void test_move_ops()
{
	// MOVEL is left-justified, MOVE is right-justified
	Fixed<6> l = "......";
	l.movel(FixedTemp("abc"));
	CHECK(std::strcmp(l.c_str(), "abc...") == 0);

	Fixed<6> r = "......";
	r.move(FixedTemp("abc"));
	CHECK(std::strcmp(r.c_str(), "...abc") == 0);

	// Source longer than the target keeps the correct end
	Fixed<3> shortL = "...";
	shortL.movel(FixedTemp("abcde"));
	CHECK(std::strcmp(shortL.c_str(), "abc") == 0);
	Fixed<3> shortR = "...";
	shortR.move(FixedTemp("abcde"));
	CHECK(std::strcmp(shortR.c_str(), "cde") == 0);

	// MOVEA replaces from a 1-based index onward; MOVEALL fills from a 1-based index onward
	Fixed<6> a = "......";
	a.movea(FixedTemp("XY"), 3);
	CHECK(std::strcmp(a.c_str(), "..XY..") == 0);

	Fixed<6> all = "......";
	all.moveall('Z', 4);
	CHECK(std::strcmp(all.c_str(), "...ZZZ") == 0);

	Fixed<6> fig = "......";
	fig.movea(BLANKS, 5);
	CHECK(std::strcmp(fig.c_str(), "....  ") == 0);

	// MOVEA from a plain array packs each element's raw bytes contiguously, starting
	// at the 1-based index, same as RPG packing an array into a field
	int nums[3] = {1, 2, 3};
	Fixed<sizeof(nums)> packedAll;
	packedAll.movea(nums);
	int unpacked[3];
	memcpy(unpacked, packedAll.overlay, sizeof(unpacked));
	CHECK(unpacked[0]==1 && unpacked[1]==2 && unpacked[2]==3);

	Fixed<sizeof(int)*2> packedTail;
	packedTail.movea(nums, 2);
	int unpackedTail[2];
	memcpy(unpackedTail, packedTail.overlay, sizeof(unpackedTail));
	CHECK(unpackedTail[0]==2 && unpackedTail[1]==3);

	// A plain char array binds to the same template (T=char, N=array size) rather than
	// going through FixedTemp's implicit conversion -- exact reference binding beats a
	// user-defined conversion. sizeof(char)==1 makes the result identical either way.
	char letters[4] = {'W','X','Y','Z'};
	Fixed<4> fromCharArray;
	fromCharArray.movea(letters);
	CHECK(std::strcmp(fromCharArray.c_str(), "WXYZ") == 0);
}

////////////////////////////////////////////////////////////////////////////////
// TESTN / TESTZ
static void test_testn_testz()
{
	CHECK(testn(' ') == 0);      // blank
	CHECK(testn('7') == 1);      // digit
	CHECK(testn('%') == -1);     // not numeric

	// Whole-field form: -1 if any non-blank character is not a digit
	CHECK(testn(FixedTemp("123")) == 2);    // index of last non-blank digit
	CHECK(testn(FixedTemp("12x")) == -1);
	CHECK(testn(FixedTemp("   ")) == 0);

	// TESTZ classifies the zoned sign-encoded characters
	CHECK(testz('&') == 1);      // positive zone
	CHECK(testz('A') == 1);
	CHECK(testz('-') == 1);      // negative zone
	CHECK(testz('J') == 1);
	CHECK(testz('1') == 0);      // plain digit is not a zone character
}

////////////////////////////////////////////////////////////////////////////////
// %ELEM, %XFOOT, SORTA, %LOOKUP over plain arrays
static void test_array_bifs()
{
	int nums[] = {5, 3, 9, 1, 7};
	CHECK(ELEM(nums) == 5);
	CHECK(std::fabs((double)XFOOT(nums) - 25.0) < 0.001);

	CHECK(LOOKUP(9, nums, 1) == 3);       // exact match at index 3
	CHECK(LOOKUP(4, nums, 1) == 0);       // absent
	CHECK(LOOKUPGT(4, nums, 1) == 1);     // first element > 4 is 5, at index 1
	CHECK(LOOKUPLT(4, nums, 1) == 2);     // first element < 4 is 3, at index 2

	SORTA(nums);
	CHECK(nums[0] == 1 && nums[1] == 3 && nums[2] == 5 && nums[3] == 7 && nums[4] == 9);

	// XFOOT over decimals must accumulate exactly
	Zoned<7,2> money[3];
	money[0].assign(__D("10.25"));
	money[1].assign(__D("20.50"));
	money[2].assign(__D("30.25"));
	CHECK(std::fabs((double)XFOOT(money) - 61.00) < 0.001);
}

////////////////////////////////////////////////////////////////////////////////
// Array<> wrapper
static void test_array_class()
{
	Array<int, 4> a;
	a[0] = 40; a[1] = 10; a[2] = 30; a[3] = 20;
	a.sorta();
	CHECK(a[0] == 10 && a[1] == 20 && a[2] == 30 && a[3] == 40);
	CHECK(a.lookup(30, 1) == 3);
}

////////////////////////////////////////////////////////////////////////////////
// Numeric increment / decrement / modulo
static void test_numeric_operators()
{
	Zoned<5,2> z = __D("10.00");
	++z;
	CHECK(std::fabs((double)z - 11.00) < 0.001);
	--z;
	CHECK(std::fabs((double)z - 10.00) < 0.001);

	Zoned<5,0> whole = 17;
	CHECK((whole % 5) == 2);

	packed(5,0) p = 17;
	CHECK((p % 5) == 2);
}

////////////////////////////////////////////////////////////////////////////////
// String BIF helpers
static void test_string_bifs()
{
	CHECK(SCANRPL("cat", "dog", "cat scat") == "dog sdog");
	CHECK(REPLACE("XY", "abcdef", 3, 2) == "abXYef");
	CHECK(REPLACE("!", "abc", 4, 0) == "abc!");

	std::vector<std::string> parts = SPLIT("a,,b,", ",");
	CHECK(parts.size() == 4);
	CHECK(parts[0] == "a" && parts[1].empty() && parts[2] == "b" && parts[3].empty());

	CHECK(XLATE("abc", "123", "cab") == "312");
	CHECK(UPPER("Abc 123") == "ABC 123");
	CHECK(LOWER("AbC 123") == "abc 123");
}

int main()
{
	test_scan();
	test_check();
	test_cat_and_cstr();
	test_move_ops();
	test_testn_testz();
	test_array_bifs();
	test_array_class();
	test_numeric_operators();
	test_string_bifs();

	if (failures == 0)
	{
		std::printf("All RPG BIF/opcode checks passed.\n");
		return 0;
	}
	std::printf("%d RPG BIF/opcode check(s) failed.\n", failures);
	return 1;
}
