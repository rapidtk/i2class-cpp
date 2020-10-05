#ifndef I2400_H
#define I2400_H

// define bool, true, and false if the compiler doesn't recognize them
// Any compiler that does not have function templates should define NO_FUNCTION_TEMPLATE
#ifdef __OS400__
# ifndef __OS400_TGTVRM__
#  define __OS400_TGTVRM__ 440
# endif
# if (__OS400_TGTVRM__<510)
//	typedef int bool;
#  define bool int
#  define true -1
#  define false 0
#  define NO_FUNCTION_TEMPLATE
# endif
#else
# define NO_PACKED
#endif

#endif

 