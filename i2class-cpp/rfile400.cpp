#include "i2class.h"
//#include <string>

#include "rfile400.h"
#include "rglobal.h"
#include <xxcvt.h>

const char *READ_ONLY="rr";
const char *READ_WRITE="rr+";
const char *WRITE_ONLY="ar";

const char COMMIT_LOCK_LEVEL_NONE='N';
const char COMMIT_LOCK_LEVEL_DEFAULT='Y';

class CRIOFileNotOpened : public CI2Err { } I2FileNotOpened;

void setIndara(_RFILE *fp);

/*
Rfile400::Rfile400(AS400 &as400, char *sFileName, Rrecord400 &format, char *openType)
 : Rfile(as400, sFileName, format)
{
	open(openType);
}
*/

// On deletion, close file
Rfile400::~Rfile400()
{
	close();
}

// Close file
char Rfile400::close()
{
	_Rclose(fp);
	return '0';
}

char Rfile400::open(const char *OpenType, int blockingFactor, char commitLockLevel)
{
//   string mode= (string)OpenType + ", commit=" + commitLockLevel;
//   fp = _Ropen(fileName, mode.c_str());
   if (commitLockLevel==COMMIT_LOCK_LEVEL_DEFAULT)
   {
   	char buf[15];
      strcpy(buf, OpenType);
      strcat(buf, ", commit=Y");
		fp = _Ropen(fileName, buf);
   }
   else
		fp = _Ropen(fileName, OpenType);
	//_Rformat(fp, record->recordName);
   if (!fp)
   	throw I2FileNotOpened;
   // If a record format has already been set, then make call to Rformat here
   if (record)
   	_Rformat(fp, record->recordName);
	return '0';
}

char Rfile400::readx()
{
	if (feedBack->num_bytes<=0)
		return '1';
	record->input();
	((Rrecord400 *)record)->recno = feedBack->rrn;
	return '0';
}

// Force end of data
void Rfile400::feod()
{
	_Rfeod(fp);
}

char Rfile400::read()
{
	Rrecord400 *rcd = (Rrecord400 *)record;
	feedBack=_Rreadn(fp, rcd->inputBuffer, rcd->inputSize, __DFT);
	return readx();
}
char Rfile400::read(Rrecord400 &format)
{
	setRecordFormat(format);
	return read();
}

void Rfile400::update()
{
	record->output();
	Rrecord400 *rcd = (Rrecord400 *)record;
	feedBack=_Rupdate(fp, rcd->outputBuffer, rcd->outputSize);
}
void Rfile400::update(Rrecord400 &format)
{
	setRecordFormat(format);
	update();
}

char Rfile400::write()
{
	record->output();
	Rrecord400 *rcd = (Rrecord400 *)record;
	feedBack=_Rwrite(fp, rcd->outputBuffer, rcd->outputSize);
	return '0';
}
char Rfile400::write(Rrecord400 &format)
{
	setRecordFormat(format);
	return write();
}

// Write directly to a specific rrn
char Rfile400::write(Rrecord400 &format, unsigned long rrn)
{
	setRecordFormat(format);
	format.output();
	feedBack=_Rwrited(fp, format.outputBuffer, format.outputSize, rrn);
	return '0';
}

void Rfile400::setRecordFormat(Rrecord400 &format)
{
	if (fp)
   	_Rformat(fp, format.recordName);
	setRecord(format);
}

char Rfile400::chain()
{
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->inputSize, __KEY_EQ, dbRcd->key,
	 dbRcd->keyLength);
	return readx();
}
char Rfile400::chain(Rrecord400 &format)
{
	setRecordFormat(format);
	return chain();
}
// Chain to a relative record number
char Rfile400::chain(Rrecord400 &format, long rrn)
{
	setRecordFormat(format);
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	feedBack=_Rreadd(fp, dbRcd->inputBuffer, dbRcd->inputSize, __DFT, rrn);
	return readx();
}

char RdbFile400::readp()
{
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	feedBack=_Rreadp(fp, dbRcd->inputBuffer, dbRcd->inputSize, __DFT);
	return readx();
}
char RdbFile400::readp(Rrecord400 &format)
{
	setRecordFormat(format);
	return readp();
}

// We can't do the READxE at the data management level, so do the record comparison here
char RdbFile400::readxe()
{
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	if (feedBack->num_bytes<=0 || memcmp(feedBack->key, dbRcd->key, dbRcd->keyLength)!=0)
		return '1';
	record->input();
	return '0';
}

char RdbFile400::reade()
{
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	/*
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->recordSize, __KEY_NEXTEQ, dbRcd->key,
	 dbRcd->keyLength);
	*/
	feedBack=_Rreadn(fp, dbRcd->inputBuffer, dbRcd->inputSize, __DFT);
	return readxe();
}
char RdbFile400::reade(Rrecord400 &format)
{
	setRecordFormat(format);
	return reade();
}
char RdbFile400::readEqual()
{
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->inputSize, __KEY_NEXTEQ, NULL, NULL);
	return readx();
}

char RdbFile400::readpe()
{
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	/*
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->inputSize, __KEY_PREVEQ, dbRcd->key,
	 dbRcd->keyLength);
	*/
	feedBack=_Rreadp(fp, dbRcd->inputBuffer, dbRcd->inputSize, __DFT);
	return readxe();
}
char RdbFile400::readpe(Rrecord400 &format)
{
	setRecordFormat(format);
	return readpe();
}
char RdbFile400::readpEqual()
{
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	feedBack=_Rreadk(fp, dbRcd->inputBuffer, dbRcd->inputSize, __KEY_PREVEQ, NULL, NULL);
	return readx();
}

char RdbFile400::setgtEOF()
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
char RdbFile400::setgt()
{
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	feedBack=_Rlocate(fp, dbRcd->key, dbRcd->keyLength, __KEY_GT | __PRIOR);
	// Check for end of file
	return setgtEOF();
}
char RdbFile400::setgt(Rrecord400 &format)
{
	setRecordFormat(format);
	return setgt();
}
// Position past a relative record number
char RdbFile400::setgt(Rrecord400 &format, long rrn)
{
	setRecordFormat(format);
	feedBack=_Rlocate(fp, NULL, rrn+1, __RRN_EQ);
	return setgtEOF();
}

char RdbFile400::setll()
{
	RdbRecord400 *dbRcd = (RdbRecord400 *)record;
	feedBack=_Rlocate(fp, dbRcd->key, dbRcd->keyLength, __KEY_GE | __PRIOR);
	// Return ON if the positioned record is equal to the key
	if (feedBack->num_bytes<=0 || memcmp(feedBack->key, dbRcd->key, dbRcd->keyLength)!=0)
		return '0';
	return '1';
}
char RdbFile400::setll(Rrecord400 &format)
{
	setRecordFormat(format);
	return setll();
}
// Position to a relative record number
char RdbFile400::setll(Rrecord400 &format, long rrn)
{
	setRecordFormat(format);
	feedBack=_Rlocate(fp, NULL, rrn, __RRN_EQ);
	return '1';
}

/*static*/ void RdbFile400::commit(AS400 system, char *cmtID)
{
	_Rcommit(cmtID);
}

/*static*/ void RdbFile400::rolbk(AS400 system)
{
	_Rrollbck();
}

// Release a record lock
void RdbFile400::unlock()
{
	_Rrlslck(fp);
}

// Delete a record
char RdbFile400::Delete()
{
	feedBack=_Rdelete(fp);
	return '0';
}
char RdbFile400::Delete(Rrecord400 &format)
{
	setRecordFormat(format);
	return Delete();
}
char RdbFile400::Delete(Rrecord400 &format, long rrn)
{
	chain(format, rrn);
	return Delete();
}


// Display file functions /////////////////////////////////////////
char RfileDspf::open(const char *openType, int blockingFactor, char commitLockLevel)
{
	char c=Rfile400::open(openType, blockingFactor, commitLockLevel);
	if (c=='0')
		setIndara(fp);
	return c;
}

void RfileDspf::acq(char *dev)
{
	_Racquire(fp, dev);
}

void RfileDspf::rel(char *dev)
{
	_Rrelease(fp, dev);
}

void RfileDspf::exfmt()
{
	record->output();
	Rrecord400 *dspRcd = (Rrecord400 *)record;
	feedBack=_Rwrread(fp, dspRcd->inputBuffer, dspRcd->inputSize,
	 dspRcd->outputBuffer, dspRcd->outputSize);
	readx();
}
void RfileDspf::exfmt(Rrecord400 &format)
{
	setRecordFormat(format);
	exfmt();
}

char RfileDspf::readc()
{
	Rrecord400 *dspRcd = (Rrecord400 *)record;
	feedBack=_Rreadnc(fp, dspRcd->inputBuffer, dspRcd->inputSize);
	return readx();
}
char RfileDspf::readc(Rrecord400 &format)
{
	setRecordFormat(format);
	return readc();
}

char RfileDspf::readx()
{
	_XXIOFB_T *iofb=_Riofbk(fp);
	_XXIOFB_DSP_ICF_T *iofbDsp=(_XXIOFB_DSP_ICF_T *)((char *)iofb + iofb->file_dep_fb_offset);
   //Zero out the existing indicators
   memset(&INKA, '0', 24);
   // This is goofy, but we have to set the INKx indicator if a function key is pressed
   // 0x31-0x3C correspond to F1-F12, 0xB1-0xBC correspond to F13-F24
   // INKA-INKL correspond to F1-F12, INKM=F13, INKN=F14,
   //  then INKP-INKY correspond to F15-F24 (INKO is skipped, who knows why)
	//if (iofbDsp->cmd_key_indic > 0) // evidently, this doesn't matter
   {
      // Translate the function key range so that it is 0x31->0x49 (24 consecutive bytes)
   	if (iofbDsp->AID_byte_indic>=0xB1)
      	iofbDsp->AID_byte_indic -= 0x74; // 0x74 = 0xB1-0X3D
   	if (iofbDsp->AID_byte_indic>=0x31 && iofbDsp->AID_byte_indic<=0x49)
      	*((char *)(&INKA + iofbDsp->AID_byte_indic - 0x31))='1';
   }
   return Rfile400::readx();
}

////////////////////////////////////////////////////////////////////////////////
// Print file stuff
char RfilePrtf::openPrtf(const char *openType, int blockingFactor, char commitLockLevel)
{
	char c=Rfile400::open(openType, blockingFactor, commitLockLevel);
	if (c=='0')
	{
		_XXOPFB_T *opfb=_Ropnfbk(fp);
		overflow=opfb->overflow_line_num;
		outputSize=opfb->pgm_record_len;
	}
	return c;
}
char RfilePrtf::open(const char *openType, int blockingFactor, char commitLockLevel)
{
	char c=openPrtf(openType, blockingFactor, commitLockLevel);
	if (c=='0')
		setIndara(fp);
	return c;
}

char RfilePrtf::checkOverflow()
{
	_XXIOFB_T *iofb=_Riofbk(fp);
	_XXIOFB_PRT_T *iofbPrt=(_XXIOFB_PRT_T *)((char *)iofb + iofb->file_dep_fb_offset);
	if (iofbPrt->cur_page_line_num > overflow)
		return '1';
	return '0';
}

char RfilePrtf::write()
{
	Rfile400::write();
	return checkOverflow();
}

//Program-described printer files
extern "OS" QCMDEXC(char *cmd, char *len);
char RfileOspec::open(const char *openType, int blockingFactor, char commitLockLevel)
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

void RfileOspec::setRecordFormat(RrecordOspec &format)
{
	setRecord(format);
	format.fp=fp;
	format.outputSize=outputSize;
	memset(format.outputBuffer, ' ', outputSize+1);
}

char RfileOspec::write()
{
	record->output();
	RrecordOspec *rcd = (RrecordOspec *)record;
	rcd->flush();
	return checkOverflow();
}

char RfileOspec::write(RrecordOspec &record)
{
	setRecordFormat(record);
	return write();
}

char RfileOspec::close()
{
	RrecordOspec *rcd = (RrecordOspec *)record;
	rcd->flush();
	return Rfile400::close();
}

void RrecordOspec::updatePage()
{
	_XXIOFB_T *iofb=_Riofbk(fp);
	_XXIOFB_PRT_T *iofbPrt=(_XXIOFB_PRT_T *)((char *)iofb + iofb->file_dep_fb_offset);
	page=iofbPrt->cur_page_count;
}

void RrecordOspec::flush()
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

void RrecordOspec::space(int rows)
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

void RrecordOspec::skip(int rows)
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

