// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TIME_HPP
#define NP1_TIME_HPP

#include "np1/simple_types.hpp"
#ifndef _WIN32
#include <sys/time.h>
#include <time.h>
#endif



namespace np1 {
namespace time {

/// Time since 1970 GMT in microseconds.
uint64_t now_epoch_usec() {
#ifdef _WIN32
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  uint64_t result = ft.dwHighDateTime;
  result <<= 32;
  result |= ft.dwLowDateTime;
  // Convert to microseconds.
  result /= 10;
  // Convert from the 1601 epoch to the 1970 one.
  result -= 11644473600000000ULL;
  return result;
#else
  struct timeval tv;
  NP1_ASSERT(gettimeofday(&tv, 0) == 0, "gettimeofday failed!");
  uint64_t result = tv.tv_sec;
  result *= 1000000;
  result += tv.tv_usec;
  return result;
#endif
}


/// Convert microseconds to other things and back.
uint64_t usec_to_msec(uint64_t usec) { return usec/1000; }
uint64_t usec_to_sec(uint64_t usec) { return usec/1000000; }

uint64_t msec_to_usec(uint64_t msec) { return msec * 1000; }
uint64_t sec_to_usec(uint64_t usec) { return usec * 1000000; }
int64_t sec_to_usec(int64_t usec) { return usec * 1000000; }

// Helpers data part extractions.
#ifdef _WIN32
void usec_to_filetime(uint64_t usec, FILETIME *ft) {
  usec *= 10;
  ft.dwHighDateTime = (DWORD)((usec & 0xffffffff00000000) >> 32);
  ft.dwLowDateTime = (DWORD)usec;
}

void usec_to_systemtime(uint64_t usec, SYSTEMTIME *st) {
  FILETIME ft;  
  usec_to_filetime(usec, &ft);
  FileTimeToSystemTime(&ft, st);
}
#else
void usec_to_tm(uint64_t usec, struct tm *result) {
  time_t sec = usec_to_sec(usec);
  gmtime_r(&sec, result);
}
#endif


/// Extract parts of the supplied time.
uint32_t extract_year(uint64_t usec) {
#ifdef _WIN32
  SYSTEMTIME st;
  usec_to_systemtime(usec, &st);
  return st.wYear;
#else
  struct tm result;
  usec_to_tm(usec, &result);
  return result.tm_year + 1900;
#endif
}


uint32_t extract_month(uint64_t usec) {
#ifdef _WIN32
  SYSTEMTIME st;
  usec_to_systemtime(usec, &st);
  return st.wMonth;
#else
  struct tm result;
  usec_to_tm(usec, &result);
  return result.tm_mon + 1;
#endif
}

uint32_t extract_day(uint64_t usec) {
#ifdef _WIN32
  SYSTEMTIME st;
  usec_to_systemtime(usec, &st);
  return st.wDay;
#else
  struct tm result;
  usec_to_tm(usec, &result);
  return result.tm_mday;
#endif
}

uint32_t extract_hour(uint64_t usec) {
#ifdef _WIN32
  SYSTEMTIME st;
  usec_to_systemtime(usec, &st);
  return st.wHour;
#else
  struct tm result;
  usec_to_tm(usec, &result);
  return result.tm_hour;
#endif
}

uint32_t extract_minute(uint64_t usec) {
#ifdef _WIN32
  SYSTEMTIME st;
  usec_to_systemtime(usec, &st);
  return st.wMinute;
#else
  struct tm result;
  usec_to_tm(usec, &result);
  return result.tm_min;
#endif
}

uint32_t extract_second(uint64_t usec) {
#ifdef _WIN32
  SYSTEMTIME st;
  usec_to_systemtime(usec, &st);
  return st.wSecond;
#else
  struct tm result;
  usec_to_tm(usec, &result);
  return result.tm_sec;
#endif
}




} // namespaces.
}



#endif
