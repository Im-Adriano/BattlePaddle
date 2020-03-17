#ifndef TIME_H
#define TIME_H

#if defined(_WIN32) || defined(WIN32)

#include <stdint.h>
#include <windows.h>


uint64_t GetTimeOfDay(void);

#endif
#endif