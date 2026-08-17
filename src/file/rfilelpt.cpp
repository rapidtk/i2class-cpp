#include "rfilelpt.h"

////////////////////////////////////////////////////////////////////////////////
// Print file stuff

char RfileLPT::open(const char *openType)
{
	fp=fopen(fileName, "w");
	return '0';
}

char RfileLPT::close()
{
	((RrecordLPT*)record)->flush();
	fclose(fp);
	return '0';
}

// Print out a numeric value
char RfileLPT::write()
{
	record->output();
   return '0';
}

char RfileLPT::write(RrecordLPT &format)
{
	setRecordFormat(format);
	return write();
}

void RfileLPT::setRecordFormat(RrecordLPT &format)
{
	Rfile::setRecord(format);
	format.fp=fp;
	memset(format.outputBuffer, ' ', sizeof(format.outputBuffer));
}

int RrecordLPT::page=1;
void RrecordLPT::space(int rows)
{
	flush();
	while (rows>0)
	{
		fputc('\n', fp);
		rows--;
		row++;
	}
}
void RrecordLPT::skip(int row)
{
	space(row);
}

void RrecordLPT::flush()
{
	if (maxColumn>0)
	{
		fprintf(fp, "%s*", outputBuffer+1, maxColumn);
		memset(outputBuffer, ' ', maxColumn+1);
		maxColumn=0;
		column=1;
	}
}




 