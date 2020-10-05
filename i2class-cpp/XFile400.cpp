#include "xport.h"
//#include <string>

#include "Xfile400.h"
#include "XGlobal.h"
#include <xxcvt.h>

const char *READ_ONLY="rr";
const char *READ_WRITE="rr+";
const char *WRITE_ONLY="ar";

const char COMMIT_LOCK_LEVEL_NONE='n';
const char COMMIT_LOCK_LEVEL_DEFAULT='y';

void setIndara(_RFILE *fp);

/*
Xfile400::Xfile400(AS400 &as400, char *sFileName, Xrecord400 &format, char *openType)
 : Xfile(as400, sFileName, format)
{
	open(openType);
}
*/

// On deletion, close file
Xfile400::~Xfile400()
{
	close();
}

// Close file
char Xfile400::close()
{
	_Rclose(fp);
	return '0';
}

char Xfile400::open(const char *OpenType, int blockingFactor, char commitLockLevel)
{
//   string mode= (string)OpenType + ", commit=" + commitLockLevel;
//   fp = _Ropen(fileName, mode.c_str());
	fp = _Ropen(fileName, OpenType);
	//_Rformat(fp, record->recordName);
	return '0';
}

char Xfile400::readx()
{
	if (feedBack->num_bytes<=0)
		return '1';
	record->input();
	((Xrecord400 *)record)->recno = feedBack->rrn;
	return '0';
}

// Force end of data
void Xfile400::feod()
{
	_Rfeod(fp);
}

char Xfile400::read()
{
	Xrecord400 *rcd = (Xrecord400 *)record;
	feedBack=_Rreadn(fp, rcd->inputBuffer, rcd->inputSize, __DFT);
	return readx();
}
char Xfile400::read(Xrecord400 &format)
{
	setRecordFormat(format);
	return read();
}

void Xfile400::update()
{
	record->output();
	Xrecord400 *rcd = (Xrecord400 *)record;
	feedBack=_Rupdate(fp, rcd->outputBuffer, rcd->outputSize);
}
void Xfile400::update(Xrecord400 &format)
{
	setRecordFormat(format);
	update();
}

char Xfile400::write()
{
	record->output();
	Xrecord400 *rcd = (Xrecord400 *)record;
	feedBack=_Rwrite(fp, rcd->outputBuffer, rcd->outputSize);
	return '0';
}
char Xfile400::write(Xrecord400 &format)
{
	setRecordFormat(format);
	return write();
}

// Write directly to a specific rrn
char Xfile400::write(Xrecord400 &format, unsigned long rrn)
{
	setRecordFormat(format);
	format.output();
	feedBack=_Rwrited(fp, format.outputBuffer, format.outputSize, rrn);
	return '0';
}

void Xfile400::setRecordFormat(Xrecord400 &format)
{
	_Rformat(fp, format.recordName);
	setRecord(format);
}

char Xfile400::chain()
{
	XdbRecord400 *dbRcd = (XdbRecord400 *)record;
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->inputSize, __KEY_EQ, dbRcd->key,
	 dbRcd->keyLength);
	return readx();
}
char Xfile400::chain(Xrecord400 &format)
{
	setRecordFormat(format);
	return chain();
}
// Chain to a relative record number
char Xfile400::chain(Xrecord400 &format, long rrn)
{
	setRecordFormat(format);
	XdbRecord400 *dbRcd = (XdbRecord400 *)record;
	feedBack=_Rreadd(fp, dbRcd->inputBuffer, dbRcd->inputSize, __DFT, rrn);
	return readx();
}

char XdbFile400::readp()
{
	XdbRecord400 *dbRcd = (XdbRecord400 *)record;
	feedBack=_Rreadp(fp, dbRcd->inputBuffer, dbRcd->inputSize, __DFT);
	return readx();
}
char XdbFile400::readp(Xrecord400 &format)
{
	setRecordFormat(format);
	return readp();
}

// We can't do the READxE at the data management level, so do the record comparison here
char XdbFile400::readxe()
{
	XdbRecord400 *dbRcd = (XdbRecord400 *)record;
	if (feedBack->num_bytes<=0 || memcmp(feedBack->key, dbRcd->key, dbRcd->keyLength)!=0)
		return '1';
	record->input();
	return '0';
}

char XdbFile400::reade()
{
	XdbRecord400 *dbRcd = (XdbRecord400 *)record;
	/*
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->recordSize, __KEY_NEXTEQ, dbRcd->key,
	 dbRcd->keyLength);
	*/
	feedBack=_Rreadn(fp, dbRcd->inputBuffer, dbRcd->inputSize, __DFT);
	return readxe();
}
char XdbFile400::reade(Xrecord400 &format)
{
	setRecordFormat(format);
	return reade();
}

char XdbFile400::readpe()
{
	XdbRecord400 *dbRcd = (XdbRecord400 *)record;
	/*
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->inputSize, __KEY_PREVEQ, dbRcd->key,
	 dbRcd->keyLength);
	*/
	feedBack=_Rreadp(fp, dbRcd->inputBuffer, dbRcd->inputSize, __DFT);
	return readxe();
}
char XdbFile400::readpe(Xrecord400 &format)
{
	setRecordFormat(format);
	return readpe();
}

char XdbFile400::setgtEOF()
{
	// If this is true, then we have positioned past the last record in the
	// access path.  For some reason, it doesn't position to the end of the file,
	// so we have to do it here
	if (feedBack->num_bytes<=0)
	{
		feedBack=_Rlocate(fp, NULL, NULL, __END);
		return '0';
	}
	return '1';
}
char XdbFile400::setgt()
{
	XdbRecord400 *dbRcd = (XdbRecord400 *)record;
	feedBack=_Rlocate(fp, dbRcd->key, dbRcd->keyLength, __KEY_GT | __PRIOR);
	// Check for end of file
	return setgtEOF();
}
char XdbFile400::setgt(Xrecord400 &format)
{
	setRecordFormat(format);
	return setgt();
}
// Position past a relative record number
char XdbFile400::setgt(Xrecord400 &format, long rrn)
{
	setRecordFormat(format);
	feedBack=_Rlocate(fp, NULL, rrn+1, __RRN_EQ);
	return setgtEOF();
}

char XdbFile400::setll()
{
	XdbRecord400 *dbRcd = (XdbRecord400 *)record;
	feedBack=_Rlocate(fp, dbRcd->key, dbRcd->keyLength, __KEY_GE | __PRIOR);
	// Return ON if the positioned record is equal to the key
	if (feedBack->num_bytes<=0 || memcmp(feedBack->key, dbRcd->key, dbRcd->keyLength)!=0)
		return '0';
	return '1';
}
char XdbFile400::setll(Xrecord400 &format)
{
	setRecordFormat(format);
	return setll();
}
// Position to a relative record number
char XdbFile400::setll(Xrecord400 &format, long rrn)
{
	setRecordFormat(format);
	feedBack=_Rlocate(fp, NULL, rrn, __RRN_EQ);
	return '1';
}

/*static*/ void XdbFile400::commit(char *cmtID)
{
	_Rcommit(cmtID);
}

/*static*/ void XdbFile400::rolbk()
{
	_Rrollbck();
}

// Release a record lock
void XdbFile400::unlock()
{
	_Rrlslck(fp);
}

// Delete a record
char XdbFile400::Delete()
{
	feedBack=_Rdelete(fp);
	return '0';
}
char XdbFile400::Delete(Xrecord400 &format, long rrn)
{
	chain(format, rrn);
	return Delete();
}


// Display file functions /////////////////////////////////////////
char XfileDspf::open(const char *openType, int blockingFactor, char commitLockLevel)
{
	char c=Xfile400::open(openType, blockingFactor, commitLockLevel);
	if (c=='0')
		setIndara(fp);
	return c;
}

void XfileDspf::acq(char *dev)
{
	_Racquire(fp, dev);
}

void XfileDspf::rel(char *dev)
{
	_Rrelease(fp, dev);
}

void XfileDspf::exfmt()
{
	record->output();
	Xrecord400 *dspRcd = (Xrecord400 *)record;
	feedBack=_Rwrread(fp, dspRcd->inputBuffer, dspRcd->inputSize,
	 dspRcd->outputBuffer, dspRcd->outputSize);
	record->input();
	//readx();
}
void XfileDspf::exfmt(Xrecord400 &format)
{
	setRecordFormat(format);
	exfmt();
}

char XfileDspf::readc()
{
	Xrecord400 *dspRcd = (Xrecord400 *)record;
	feedBack=_Rreadnc(fp, dspRcd->inputBuffer, dspRcd->inputSize);
	return readx();
}
char XfileDspf::readc(Xrecord400 &format)
{
	setRecordFormat(format);
	return readc();
}

////////////////////////////////////////////////////////////////////////////////
// Print file stuff
char XfilePrtf::openPrtf(const char *openType, int blockingFactor, char commitLockLevel)
{
	char c=Xfile400::open(openType, blockingFactor, commitLockLevel);
	if (c=='0')
	{
		_XXOPFB_T *opfb=_Ropnfbk(fp);
		overflow=opfb->overflow_line_num;
		outputSize=opfb->pgm_record_len;
	}
	return c;
}
char XfilePrtf::open(const char *openType, int blockingFactor, char commitLockLevel)
{
	char c=openPrtf(openType, blockingFactor, commitLockLevel);
	if (c=='0')
		setIndara(fp);
	return c;
}

char XfilePrtf::checkOverflow()
{
	_XXIOFB_T *iofb=_Riofbk(fp);
	_XXIOFB_PRT_T *iofbPrt=(_XXIOFB_PRT_T *)((char *)iofb + iofb->file_dep_fb_offset);
	if (iofbPrt->cur_page_line_num > overflow)
		return '1';
	return '0';
}

char XfilePrtf::write()
{
	Xfile400::write();
	return checkOverflow();
}

//Program-described printer files
extern "OS" QCMDEXC(char *cmd, char *len);
char XfileOspec::open(const char *openType, int blockingFactor, char commitLockLevel)
{
	// Override the print file so that it can use form control characters
	char ovrCmd[55]="OVRPRTF            CTLCHAR(*FCFC   ) PAGESIZE(*N     )";
	char cmdLen[8]={0x00, 0x00, 0x00, 0x00, 0x54, 0x00, 0x00, 0x0F}; // Packed<15,5> 54
	char *ovrFile=ovrCmd+8;
	int cpyLen=strlen(fileName);
	memcpy(ovrFile, fileName, cpyLen);
	QXXITOZ((unsigned char *)(ovrCmd+49), 4, 0, outputSize+1);
	QCMDEXC(ovrCmd, cmdLen);
	return openPrtf(openType, blockingFactor, commitLockLevel);
}

void XfileOspec::setRecordFormat(XrecordOspec &format)
{
	setRecord(format);
	format.fp=fp;
	format.outputSize=outputSize;
	memset(format.outputBuffer, ' ', outputSize+1);
}

char XfileOspec::write()
{
	record->output();
	XrecordOspec *rcd = (XrecordOspec *)record;
	rcd->flush();
	return checkOverflow();
}

char XfileOspec::write(XrecordOspec &record)
{
	setRecordFormat(record);
	return write();
}

char XfileOspec::close()
{
	XrecordOspec *rcd = (XrecordOspec *)record;
	rcd->flush();
	return Xfile400::close();
}

void XrecordOspec::updatePage()
{
	_XXIOFB_T *iofb=_Riofbk(fp);
	_XXIOFB_PRT_T *iofbPrt=(_XXIOFB_PRT_T *)((char *)iofb + iofb->file_dep_fb_offset);
	page=iofbPrt->cur_page_count;
}

void XrecordOspec::flush()
{
	if (maxColumn>0)
	{
		//*fcBuffer=0x01; // Print without spacing
		*outputBuffer='+'; // Print without spacing
		_Rwrite(fp, outputBuffer, outputSize+1);
		memset(outputBuffer, ' ', outputSize+1);
		maxColumn=0;
		column=1;
		updatePage();
	}
}

void XrecordOspec::space(int rows)
{
	flush();
/*
	rows--;
	while (rows>0)
	{
		_Rwrite(fp, fcBuffer, outputSize);
		rows--;
		row++;
	}
*/
	while (rows>0) {
	/*
		char fc=3+MIN(rows, 3)*8; // 11=space1, 19=space2, 27=space3
		*fcBuffer=fc; // Print without spacing
		_Rwrite(fp, fcBuffer, 1);
	*/
		switch (MIN(rows, 3)) {
		case 2:
			*outputBuffer='0'; // double spacing
			break;
		case 3:
			*outputBuffer='-'; // triple spacing
			break;
		default:
			*outputBuffer=' ';
		}
		_Rwrite(fp, outputBuffer, outputSize+1);
		rows=rows-3;
	};
	updatePage();
}

void XrecordOspec::skip(int rows)
{
	flush();
/*
	_XXIOFB_T *iofb=_Riofbk(fp);
	_XXIOFB_PRT_T *iofbPrt=(_XXIOFB_PRT_T *)((char *)iofb + iofb->file_dep_fb_offset);
	while (iofbPrt->cur_page_line_num > rows)
	{
		_Rwrite(fp, fcBuffer, recordSize);
		iofb=_Riofbk(fp);
		iofbPrt=(_XXIOFB_PRT_T *)((char *)iofb + iofb->file_dep_fb_offset);
	};
*/
	_XXIOFB_T *iofb=_Riofbk(fp);
	_XXIOFB_PRT_T *iofbPrt=(_XXIOFB_PRT_T *)((char *)iofb + iofb->file_dep_fb_offset);
	if (iofbPrt->cur_page_line_num > rows)
	{
		//*fcBuffer=0x89; // Skip to channel 1 (first line on new page)
		*outputBuffer='1'; // Skip to channel 1 (first line on new page)
		_Rwrite(fp, outputBuffer, outputSize+1);
		space(rows-1);
	}
	else
		space(rows-iofbPrt->cur_page_line_num);
}

