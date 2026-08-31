// ILLUSTRATIVE ONLY -- not compiled or run anywhere in this dev environment.
//
// RdbFile400/Rfile400 (src/file/ibmi/rfile400.h/.cpp) wrap the real IBM i ILE C
// "recio.h" record-level I/O APIs (_Ropen/_Rreadn/_Rreadk/_Rlocate/_Rwrite/_Rupdate/...),
// which only exist when compiling natively on IBM i (__OS400__) with the ILE C/C++
// compiler. There is no way to compile, link, or run this file on Windows or Linux --
// <recio.h> and the record-level I/O runtime it depends on simply don't exist off IBM i.
//
// This is written to mirror the exact API surface in rfile400.h (verified by reading that
// header directly) and the pattern real generated RPG-to-C++ conversions use for keyed
// database access: chain (CHAIN), read/reade (READ/READE), readp/readpe (READP/READPE),
// setll/setgt (SETLL/SETGT), write/update (WRITE/UPDATE). If/when this is built on real
// IBM i hardware, treat any compile errors here as bugs to fix, not as expected -- unlike
// the rest of this repo's tests, this one has never actually been built.
#ifdef __OS400__

#include <cstdio>

#include "i2class.h"
#include "ibmi/rfile400.h"

static int failures = 0;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::fprintf(stderr, "FAIL: %s (rfile400_smoke.cpp:%d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

// A minimal externally-described record format for a keyed physical file CUSTMAST
// (CUSTNO CHAR(6) key, NAME CHAR(30), BALANCE ZONED(9,2)) -- real generated code would
// overlay these fields directly onto inputBuffer/outputBuffer rather than duplicating
// storage, but this keeps the illustration self-contained.
class CustmastRecord : public RdbRecord400
{
public:
	CustmastRecord() : RdbRecord400((char *)"CUSTMAST")
	{
		key = &custno;
		keyLength = custno.len();
		keyCount = 1;
		inputBuffer = outputBuffer = &custno;
		inputSize = outputSize = custno.len() + name.len() + balance.len();
	}

	// Real generated code copies fields to/from the raw record buffer here; custno/name/
	// balance already overlay that buffer directly in this simplified illustration, so
	// there's nothing to transfer.
	void input() override {}
	void output() override {}

	Fixed<6> custno;
	Fixed<30> name;
	Zoned<9,2> balance;
};

int main()
{
	AS400 as400System((char *)"*LOCAL", (char *)"", (char *)"");
	RdbFile400 custmast(as400System, (char *)"CUSTMAST");
	CustmastRecord rec;

	bool opened = true;
	try
	{
		custmast.open(READ_WRITE, 0, COMMIT_LOCK_LEVEL_NONE);
	}
	catch (const CI2ErrFile &err)
	{
		std::fprintf(stderr, "open() threw CI2ErrFile: %s\n", err.message);
		opened = false;
	}
	CHECK(opened);

	// CHAIN CUSTNO CUSTMAST
	rec.custno = "000123";
	bool found = custmast.chain(rec);
	if (found)
	{
		std::printf("Found CUSTNO=000123, NAME=%s, BALANCE=%.2f\n",
			rec.name.c_str(), (double)rec.balance);

		// UPDATE CUSTMAST
		rec.balance.assign((double)rec.balance + 100.00);
		custmast.update(rec);
	}
	else
	{
		// WRITE CUSTMAST
		rec.name = "New Customer";
		rec.balance.assign(0.00);
		CHECK(custmast.write(rec));
	}

	// SETLL CUSTNO CUSTMAST; DOU %EOF(CUSTMAST); READE CUSTNO CUSTMAST; ENDDO
	rec.custno = "000100";
	custmast.setll(rec);
	int readCount = 0;
	while (custmast.reade(rec))
		++readCount;
	std::printf("READE loop visited %d record(s) starting at CUSTNO=000100\n", readCount);

	custmast.close();

	if (failures == 0)
	{
		std::printf("All RdbFile400 smoke checks passed.\n");
		return 0;
	}
	std::printf("%d RdbFile400 smoke check(s) failed.\n", failures);
	return 1;
}

#endif // __OS400__
