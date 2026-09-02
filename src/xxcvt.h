#ifndef XXCVT_H
#define XXCVT_H

/// @file xxcvt.h
/// @brief This header and the accompanying `xxcvt.cpp` are a thin compatibility
/// layer to add zoned/packed decimal support on platforms other than IBM i
/// where xxcvt.h is part of the operating system QSYSINC/H includes.
/// @note There is a `/legacy/xxcvt.hpp` which is a renamed copy of the IBM i
/// version of `xxcvt.h` and what this copy was based on at the time.

extern "C" {
/// @brief Convert zoned to integer [QXXZTOI](https://www.ibm.com/docs/en/ssw_ibm_i_74/rtref/qxxztoi.htm)
int QXXZTOI(unsigned char *zptr, int digits, int fraction);

/// @brief Convert zoned to double [QXXZTOD](https://www.ibm.com/docs/en/ssw_ibm_i_74/rtref/qxxztod.htm)
double QXXZTOD(unsigned char *zptr, int digits, int fraction);

/// @brief Convert double to zoned [QXXDTOZ](https://www.ibm.com/docs/en/ssw_ibm_i_74/rtref/qxxdtoz.htm)
void QXXDTOZ(unsigned char *zptr, int digits, int fraction, double value);

/// @brief Convert integer to zoned [QXXITOZ](https://www.ibm.com/docs/en/ssw_ibm_i_74/rtref/qxxitoz.htm)
void QXXITOZ(unsigned char *zptr, int digits, int fraction, int value);
}

#endif