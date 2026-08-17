#include <stdio.h>
class XrecordPrint : public Xrecord
{
friend class XfilePrint;
public:
	XrecordPrint() {memset(buf, ' ', sizeof(buf)); maxColumn=0; }
	static void edit(char *buf, const NumericTemp &n, const char *edtWrd);
	void print(const FixedTemp &f, int col=0);
	void print(const NumericTemp &n, int col=0, const char *edtWrd=NULL);
//	void printl(Fixed &f, int col=0);
//	void printl(Numeric &n, int col=0, char *edtWrd=NULL);

protected:
   virtual void flush()=0;
	void setMaxColumn(int &col);

protected:
	char	buf[378];
   int	maxColumn;
	int	column, row;
};

class XrecordLPT : public XrecordPrint
{
friend class XfileLPT;
protected:
   void flush();
   void space(int rows=1);

private:
	FILE *fp;
};

class XfilePrint : public Xfile
{
public:
   XfilePrint(AS400 &as400, char *sFileName) : Xfile(as400, sFileName)
   	{column=0; row=0; }
   ~XfilePrint() {close(); }
	virtual void write()=0;
protected:
   void lsetRecordFormat(XrecordPrint &format);

private:
	int	column, row, maxColumn;
};

class XfileLPT : public XfilePrint
{
public:
   XfileLPT(AS400 &as400, char *sFileName) : XfilePrint(as400, sFileName) {}
   void setRecordFormat(XrecordLPT &format);
	void write();
   char close();
   char open();

private:
	FILE *fp;
};


