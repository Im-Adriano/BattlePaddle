#if defined(_WIN32) || defined(WIN32)

#ifndef UTILITIES_TIME_TIME_H_
#define UTILITIES_TIME_TIME_H_

#include <stdint.h>
#include <windows.h>


uint64_t GetTimeOfDay(void);


#endif

#endif