#ifndef XXCVT_H
#define XXCVT_H

extern "C" {
// Convert zoned to integer
int QXXZTOI(unsigned char *zptr, int digits, int fraction);

// Convert zoned to double
double QXXZTOD(unsigned char *zptr, int digits, int fraction);

// Convert double to zoned
void QXXDTOZ(unsigned char *zptr, int digits, int fraction, double value);

// Convert integer to zoned
void QXXITOZ(unsigned char *zptr, int digits, int fraction, int value);
}

#endif