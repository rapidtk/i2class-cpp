#include "Node.h"
#include "clp_tab.cpp.h" // THE CLP/RPG_TAB.CPP IDENTS MUST MATCH!

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(a,b) a<=b ? a : b

extern "C" char * yytext;
extern "C" char *GET_NFUNC(char *identifier, bool &isClass);
extern char FLDS_;
char nodeBuf[1024];

// Node
Node::Node() : _isBracketed(false) {}

// NodeID
NodeId::NodeId(int type, char *value)
{
   _type = type;
	// Translate BLANK to BLANKS, ZERO to ZEROS, *MONTH/*DAY to UMONTH/UDAY
   if (strcmp(value, "BLANK")==0)
   	value="BLANKS";
   else if (strcmp(value, "ZERO")==0)
   	value="ZEROS";
   else if (strcmp(value, "MONTH")==0)
   	value="UMONTH";
   else if (strcmp(value, "DAY")==0)
   	value="UDAY";
   _value = new char[strlen(value)+1]; // Add length for null-terminator
   strcpy(_value, value);
   if (_type != IDENTIFIER)
   {
   	_numFunc="";
      _isClass=false;
   }
   else
   {
	   char *n=GET_NFUNC(_value, _isClass);
      _numFunc = new char[strlen(n)];
		strcpy(_numFunc, n);
   }
}
NodeId::~NodeId()
{
   delete[] _value;
   delete[] _numFunc;
}
char *NodeId::printNode(char *buf)
{
	char *b=buf; // for debugging
	int i = sprintf(buf, "%s", _value);
	buf += i;
   return buf;
}

// NodeProcedure
NodeProcedure::NodeProcedure(char *value, NodeOpr *arg)
: NodeId(PROCEDURE, value)
{
   _arg = arg;
}
NodeProcedure::~NodeProcedure()
{
   delete _arg;
}
char *NodeProcedure::printNode(char *buf)
{
	char *b=buf;  // for debugging
	buf=NodeId::printNode(buf);
   *buf++='(';
   // There may be no parameters e.g. funct()...
   if (_arg)
   {
	   buf=_arg->printNode(buf);
      // If this is a BIF, assume that the return type is the same as the first parameter
      _numFunc = _arg->_numFunc;
      _isClass = _arg->_isClass;
   }
   *buf++=')';
   const int LAST_INT=0;
   // Some BIFs switch the return type.  Most of them are listed here
   // TODO: add support for DATE BIFs
   const char *bif[]={"check", "checkr", "days", "diff", "elem",
    "elem", "lookup", "scan", "xfoot", "editc", "editflt", "editw"};
   for (int i=0; i<sizeof(bif)/sizeof(bif[0]); i++)
   {
      // Only check first-six positions so that we can match LOOKUPxx
   	if (strncmp(bif[i], _value, 6)==0) // 6=len('lookup')
      {
      	if (i<=LAST_INT)
         {
         	_numFunc="int";
            _isClass=false;
         }
         else
         {
         	_numFunc="fixed";
            _isClass=true;
         }
         break;
      }
   }
   return buf;
}

// NodeArray
NodeArray::NodeArray(char *value, NodeOpr *arg)
: NodeProcedure(value, arg)
{
}
char *NodeArray::printNode(char *buf)
{
	char *b=buf;  // for debugging
	buf=NodeId::printNode(buf);
   *buf++='[';
   buf=_arg->printNode(buf);
   *buf++=']';
   return buf;
}

// NodeOpr
NodeOpr::NodeOpr(int type, Node *arg1, Node *arg2)
   {
      _type = type;
      _arg1 = arg1;
      _arg2 = arg2;
   }
NodeOpr::~NodeOpr()
{
   delete _arg1;
   if (_arg2)
      delete _arg2;
}
char *NodeOpr::printNode(char *buf)
{
	char *b=buf; // for debugging

   // For unary minus (UMINUS, e.g. -10), write it out here
   if (_type==UMINUS)
   	*buf++='-';
   // Not operator
   else if (_type=='!')
   	*buf++=_type;


	// Get node string
   const char cOp[]={'=','+','-','*','/',EQUALS,GE,LE,'>','<',NE,'\0'};
#define COMPARE_TO 6
   const char *javaop[]={"assign","add","subtract","multiply","divide","equals","compareTo"};
   char *op=(char *)strchr(cOp, _type);

   char oprBuffer[256];
   *_arg1->printNode(oprBuffer)='\0'; // Add null string to returned buffer
   // Assume that this node type is the same as the left leaf type,
   // unless this is a comparison function, then the node type is null
	// if (i<(COMPARE_TO-1))
   {
      _numFunc = _arg1->_numFunc;
      _isClass = _arg1->_isClass;
   }

   // !!! THIS ORDER MATTERS! THIS HAS TO MATCH COP[COMPARE_TO]
	const char sOp[]={GE,LE,'>','<',NE,EQUALS,AND,OR,'\0'};
   const char *jsOp[]={">=", "<=", ">", "<", "!=", "=="," && ", " || "};

   bool native=(!_isClass || (!op) ||
    // If no numFunc is defined, then this must be a string literal
    (_type=='+' && (!*_numFunc || strcmp(_numFunc,"fixed")==0)));
   // Print out enclosing () if they existed in the original source (YACC strips them)
   if (_isBracketed && native)
   	*buf++='(';

   // Write out left leaf
   buf += sprintf(buf, "%s", oprBuffer);

   // Write out operator

   // If the left leaf is a class type, then translate any of the built-in operators
   // to their Java type
   if (!native && op)
      buf += sprintf(buf, ".%s(", javaop[MIN(op-cOp, COMPARE_TO)]);
   else
   {
     	// Translate the 'special' types (>1 character)
      op=(char *)strchr(sOp, _type);
      if (op)
         buf += sprintf(buf, "%s", jsOp[op-sOp]);
      // Deal with TCAT and BCAT for CL: -> var.trimr()[+" " for BCAT]+
      else if (_type==TCAT || _type==BCAT)
      {
      	buf += sprintf(buf, "%s", ".trimr()");
         if (_type==BCAT)
   	   	buf += sprintf(buf, "%s", "+\" \"");
         *buf++ = '+';
      }
      else
      {
         // If this is just a 1-character operator, dump it directly (UMINUS is dealt with, above)
         if (_type != UMINUS && _type != '!')
	         *buf++ = _type;
      }
   }

   // Write out right leaf, if it exists
   if (_arg2)
   {
      *_arg2->printNode(oprBuffer)='\0'; // Add closing null
		// If the left leaf is a character value, and this is the special
      // value ON or OFF, then change it to '1' or '0' for java
      if (FLDS_=='J' && native)
      {
         char *o=NULL;
         if (strncmp(b,"IN",2)==0)
         {
            if (strcmp(oprBuffer, "'0'")==0)
               o="OFF";
            else if (strcmp(oprBuffer, "'1'")==0)
               o="ON";
         }
         else if (strcmp(_numFunc,".charAt(0)")==0)
         {
            if (strcmp(oprBuffer, "ON")==0)
               o="'1'";
            else if (strcmp(oprBuffer, "OFF")==0)
               o="'0'";
            else if (strcmp(oprBuffer, "BLANKS")==0)
               o="' '";
         }
         if (!o)
            goto noO;
         buf += sprintf(buf, "%s", o);
      }
      else
      {
      noO:
         // If the right leaf is an expression, we need to bracket it if we are
         // going to apply numFunc, see below, to it
         // If this is a procedure list, the type of the left leaf means nothing
         bool leftNative = !_isClass && (_type != ',' && _type!=OR && _type!=AND);
         if (leftNative && _arg2->_isClass && _arg2->_type != IDENTIFIER)
            *buf++='(';
         // Write out actual text
         buf += sprintf(buf, "%s", oprBuffer);
         if (leftNative && _arg2->_isClass && _arg2->_type != IDENTIFIER)
            *buf++=')';
         // If the left leaf is a non-class type, and this is a class type, then
         // we have to use the numFunc to translate it
         if (leftNative && _arg2->_isClass)
         {
            // For assignment ('='), we want to always use the left node
            if (*_arg2->_numFunc && _type!='=')
               buf += sprintf(buf, "%s", _arg2->_numFunc);
            // If the right leaf is a 'type-less' special value, then use the value of the left leaf
            else
               buf += sprintf(buf, "%s", _arg1->_numFunc);
         }
      }
   }
   // Right out closing ')' if the original expression was bracketed, see above...
   if (_isBracketed | !native)
   	*buf++=')';
	// If this is a CompareTo, then add 'OP 0' to end of clause
   if (!native && (op-cOp) >= COMPARE_TO)
   	buf += sprintf(buf, "%s0", jsOp[op-cOp-COMPARE_TO]);
   return buf;
}


// Global Node functions
NodeId *id(int type, char *value)
{
    NodeId *p = new NodeId(type, value);
    return p;
}
NodeOpr *opr(int type, Node *arg1, Node *arg2)
{
    NodeOpr *p = new NodeOpr(type, arg1, arg2);
    return p;
}
NodeProcedure *procedure(char *value, Node *arg)
{
    NodeProcedure *p = new NodeProcedure(value, (NodeOpr*)arg);
    return p;
}
NodeArray *array(char *value, Node *arg)
{
    NodeArray *p = new NodeArray(value, (NodeOpr*)arg);
    return p;
}

void walkResults(Node *node)
{
	*(node->printNode(nodeBuf))='\0';  // Add closing NULL to returned buffer
#ifdef _DEBUG
	puts(nodeBuf);
#endif
}



