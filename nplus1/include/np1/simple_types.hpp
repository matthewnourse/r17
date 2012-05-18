// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_SIMPLE_TYPES
#define NP1_SIMPLE_TYPES


/* For rand_s. */
#ifdef _WIN32
#define _CRT_RAND_S
#include <stdlib.h>
#endif


#include <stdint.h>

/* For size_t. */
#include <string.h>


/* Boolean stuff. */
#ifdef _WIN32
#include <windows.h>
typedef SSIZE_T ssize_t;

/* MSDN says that a DWORD is always 32 bits long. */
#define NP1_DWORD_MAX ((DWORD)0xffffffff)

#else  // _WIN32
#include <unistd.h> /* For ssize_t */
#endif

// My kingdom for a compiler-supplied SSIZE_T_MAX.
#define NP1_SIZE_T_MAX ((size_t)-1)
#define NP1_SSIZE_T_MAX ((ssize_t)(NP1_SIZE_T_MAX/2))

#define NP1_TIME_T_MAX ((time_t)0x7fffffff)

#endif
