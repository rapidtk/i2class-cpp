#include <vcl.h>
#pragma hdrstop

#include "rfileADO.h"

const char *READ_ONLY="I";
const char *READ_WRITE="U";
const char *WRITE_ONLY="W";

const char COMMIT_LOCK_LEVEL_NONE='n';
const char COMMIT_LOCK_LEVEL_DEFAULT='y';


RfileADO::RfileADO(AS400 &as400, czstring sFileName): Rfile(as400, sFileName)
{
   fp = new TADOTable(NULL);
	fp->ConnectionString = (AnsiString)"Provider=IBMDA400; Data Source=" + server;
   fp->TableName = fileName;
   fp->IndexName = fp->TableName;
	// All of these properties have to be set to enable 'Seek'
   fp->CursorLocation = clUseServer;
   fp->CursorType = ctDynamic;
   fp->LockType = ltReadOnly;
   fp->TableDirect = true;
};

RfileADO::~RfileADO()
{
	close();
   delete fp;
}

// Initialize the class to point to the (already allocated) structure
void RfileADO::close()
{
   fp->Active = false;
}

void RfileADO::open(czstring OpenType, int blockingFactor, char commitLockLevel)
{

   fp->ReadOnly = (*OpenType=='I'); //I=Input=Read only
	fp->Active = true;
}

void RfileADO::setRecordFormat(RrecordADO &format)
{
	setRecord(format);
	format.fp = fp;
}

bool RfileADO::readx()
{
   if ((direction=='F' && fp->Eof) || (direction=='B' && fp->Bof))
   	return false;
	record->input();
   return true;
}
bool RfileADO::read()
{
   if (direction=='F' || direction=='C')
      fp->Next();
   direction='F';
   return readx();
}
bool RfileADO::read(RrecordADO &format)
{
	setRecordFormat(format);
	return read();
}

/* Implemented like RdbFile400 */
void RfileADO::update()
{
	record->output();
}
void RfileADO::update(RrecordADO &format)
{
	setRecordFormat(format);
	update();
}

bool RfileADO::write()
{
   // Read the current record values
   int fldCount=fp->Fields->Count;
   Variant *v = new Variant[fldCount];
   for (int i=0; i<fldCount; i++)
   	v[i] = fp->Fields->Fields[i]->AsVariant;
   // Add a new record to the dataset and make it the current record
	fp->Append();
	// Now set the current values from the old values
   for (int i=0; i<fldCount; i++)
   	fp->Fields->Fields[i]->Value = v[i];
   delete[] v;

	record->output();
	return true;
}
bool RfileADO::write(RrecordADO &format)
{
	setRecordFormat(format);
	return write();
}

/* Write directly to a specific rrn
char RfileADO::write(RrecordADO &format, unsigned long rrn)
{
	setRecordFormat(format);
	format.output();
	feedBack=_Rwrited(fp, format.outputBuffer, format.outputSize, rrn);
	return '0';
}
*/

char RfileADO::chain()
{
	direction='C';
	RrecordADO *dbRcd = (RrecordADO *)record;
   Variant *vKey=dbRcd->getVariantKey();
   Variant vSeek=VarArrayOf(vKey, dbRcd->keyCount-1);
   bool seeked=fp->Seek(vSeek, soFirstEQ);
	if (!seeked)
   	return false;
	return readx();
}
bool RfileADO::chain(RrecordADO &format)
{
	setRecordFormat(format);
	return chain();
}
// Chain to a relative record number
bool RfileADO::chain(RrecordADO &format, long rrn)
{
	setRecordFormat(format);
   fp->RecNo = rrn;
	return readx();
}

/* Implemented like RdbFile400 */
bool RfileADO::readp()
{
   if (direction!='F')
      fp->Prior();
   direction='B';
   return readx();
}
bool RfileADO::readp(RrecordADO &format)
{
	setRecordFormat(format);
	return readp();
}

// We can't do the READxE at the data management level, so do the record comparison here
bool RfileADO::readxe()
{
	RrecordADO *dbRcd = (RrecordADO *)record;
   Variant *variantKey=dbRcd->getVariantKey();
   for (int i=0; i<dbRcd->keyCount; i++)
   	if (fp->IndexFields[i]->Value != variantKey[i])
      	return false;
   return readx();
}

bool RfileADO::reade()
{
   if (!read())
   	return false;
   return readxe();
}
bool RfileADO::reade(RrecordADO &format)
{
	setRecordFormat(format);
	return reade();
}

/*
char RfileADO::readEqual()
{
	RrecordADO *dbRcd = (RrecordADO *)record;
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->inputSize, __KEY_NEXTEQ, NULL, NULL);
	return readx();
}
*/

bool RfileADO::readpe()
{
   if (!readp())
   	return false;
   return readxe();
}
bool RfileADO::readpe(RrecordADO &format)
{
	setRecordFormat(format);
	return readpe();
}
/*
char RfileADO::readpEqual()
{
	RrecordADO *dbRcd = (RrecordADO *)record;
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->inputSize, __KEY_PREVEQ, NULL, NULL);
	return readx();
}
*/

bool RfileADO::setgt()
{
	direction=' ';
	RrecordADO *dbRcd = (RrecordADO *)record;
   Variant *vKey=dbRcd->getVariantKey();
   Variant vSeek=VarArrayOf(vKey, dbRcd->keyCount-1);
	// We can't use soAfter here because we can have multiple keys with the same
   // value.  soAfter positions just after the first key with that value, not
   // after all keys with the same value.
   //bool seeked=fp->Seek(vSeek, soAfter);
   bool seeked=fp->Seek(vSeek, soAfter);
	if (seeked)
   	return true;

   // SHOULDN'T HAVE TO DO THIS!!!  BUG IN ADO
   fp->Close();
   fp->Open();

   fp->Last();
   return false;
}
bool RfileADO::setgt(RrecordADO &format)
{
	setRecordFormat(format);
	return setgt();
}
// Position past a relative record number
bool RfileADO::setgt(RrecordADO &format, long rrn)
{
	fp->RecNo = rrn+1;
   return readx();
}

bool RfileADO::setll()
{
	direction=' ';
	RrecordADO *dbRcd = (RrecordADO *)record;
   Variant *variantKey=dbRcd->getVariantKey();
   bool seeked=fp->Seek(VarArrayOf(variantKey, dbRcd->keyCount-1), soFirstEQ);
	if (seeked)
   	return true;
   seeked=fp->Seek(VarArrayOf(variantKey, dbRcd->keyCount-1), soAfterEQ);
   if (!seeked)
   	direction='F';
   return false;
}
bool RfileADO::setll(RrecordADO &format)
{
	setRecordFormat(format);
	return setll();
}
// Position to a relative record number
bool RfileADO::setll(RrecordADO &format, long rrn)
{
	direction=' ';
   fp->RecNo=rrn;
	return true;
}

int RrecordADO::getInt(int columnNumber)
{
   int i=fp->Fields->Fields[columnNumber-1]->AsInteger;
   return i;
}

double RrecordADO::getDouble(int columnNumber)
{
   double d=fp->Fields->Fields[columnNumber-1]->AsFloat;
   return d;
}

char RrecordADO::getChar(int columnNumber)
{
   AnsiString A=fp->Fields->Fields[columnNumber-1]->AsString;
	char c=A[1];
	return c;
}

/*
bool RrecordADO::copyMem(void *str, int columnNumber, int strLen)
{
   AnsiString A=fp->Fields->Fields[columnNumber-1]->AsString;
   memcpy(str, A.c_str(), strLen);
	return true;
}
*/
AnsiString RrecordADO::getAnsiString(int columnNumber)
{
   AnsiString A=fp->Fields->Fields[columnNumber-1]->AsString;
   return A;
}

