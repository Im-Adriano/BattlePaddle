/*
 * Time
 *
 * For future use, if a timestamps are needed in command responses.
 * A unified way of keeping the time since windows and linux are different.
 */
#ifndef TIME_H
#define TIME_H

#if defined(_WIN32) || defined(WIN32)

#include <stdint.h>
#include <windows.h>


uint64_t GetTimeOfDay(void);

#endif
#endif