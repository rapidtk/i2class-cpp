#ifndef XGLOBAL_H
#define XGLOBAL_H
typedef Indicator IND;
//typedef char IND;
extern IND	XIN[99];
extern IND	&IN01,&IN02,&IN03,&IN04,&IN05,&IN06,&IN07,&IN08,&IN09,&IN10;
extern IND	&IN11,&IN12,&IN13,&IN14,&IN15,&IN16,&IN17,&IN18,&IN19,&IN20;
extern IND	&IN21,&IN22,&IN23,&IN24,&IN25,&IN26,&IN27,&IN28,&IN29,&IN30;
extern IND	&IN31,&IN32,&IN33,&IN34,&IN35,&IN36,&IN37,&IN38,&IN39,&IN40;
extern IND	&IN41,&IN42,&IN43,&IN44,&IN45,&IN46,&IN47,&IN48,&IN49,&IN50;
extern IND	&IN51,&IN52,&IN53,&IN54,&IN55,&IN56,&IN57,&IN58,&IN59,&IN60;
extern IND	&IN61,&IN62,&IN63,&IN64,&IN65,&IN66,&IN67,&IN68,&IN69,&IN70;
extern IND	&IN71,&IN72,&IN73,&IN74,&IN75,&IN76,&IN77,&IN78,&IN79,&IN80;
extern IND	&IN81,&IN82,&IN83,&IN84,&IN85,&IN86,&IN87,&IN88,&IN89,&IN90;
extern IND	&IN91,&IN92,&IN93,&IN94,&IN95,&IN96,&IN97,&IN98,&IN99;
#define IN XIN
extern IND INOF, INLR, INL1, INL2, INL3, INL4, INL5, INL6, INL7, INL8, INL9;

#define RETURN if (INLR!=ON) return 0; exit(0)

#ifndef DATFMT
	#define DATFMT "*MDY"
#endif
extern FmtDate<6>	XUDATE;
extern FmtDate<8>	XDATE;

#define UDATE XUDATE
#define DATE XDATE
#endif


