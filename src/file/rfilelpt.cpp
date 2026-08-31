#include "rfilelpt.h"


void RfileLPT::open(const char *openType)
{
	fp=fopen(fileName, "w");
	if (!fp)
		throw CI2ErrFile("Failed to open print output file");
}

void RfileLPT::close()
{
	((RrecordLPT*)record)->flush();
	fclose(fp);
}

// Print out a numeric value
bool RfileLPT::write()
{
	record->output();
   return true;
}

bool RfileLPT::write(RrecordLPT &format)
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
		fprintf(fp, "%.*s", maxColumn, outputBuffer+1);
		memset(outputBuffer, ' ', maxColumn+1);
		maxColumn=0;
		column=1;
	}
}




 