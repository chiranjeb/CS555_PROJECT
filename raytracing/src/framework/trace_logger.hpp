#pragma once
#include <iostream>
#include <time.h>
#include <iomanip>
#include <sstream>

/// Set this to 1 to turn off all the DEBUG STREAMS.
#define RELEASE_BUILD 1
#define VERBOSE_BUILD 0
#define DEBUG_BUILD 0
#define ENABLE_TRANSPORT_DEBUG_TRACE 0
#define ENABLE_APPLICATION_DEBUG_TRACE 1
#define ENABLE_WIRE_MSG_DEBUG_TRACE 0

/// Get a timestamp
extern std::ostream& TimeStamp(std::ostream& ostr);

/// TODO: consider changing std::cerr to a fstream to output to log
#if RELEASE_BUILD == 1
#define RELEASE_TRACE(x) {std::stringstream str;  TimeStamp(str) << x  ;  std::cerr << str.str() << std::endl;}
#else
#define RELEASE_TRACE(x)
#endif


#if VERBOSE_BUILD == 1
#define DEBUG_TRACE_VERBOSE(x) {std::stringstream str;  str << x  ;  std::cerr << str.str();}
#else
#define DEBUG_TRACE_VERBOSE(x)
#endif
 
#if DEBUG_BUILD == 1
#define DEBUG_TRACE(x) {std::stringstream str;  TimeStamp(str) << x  ;  std::cerr << str.str() << std::endl;}
#else
/// Goal is to take these traces out in compile time to eliminate runtime cost
#define DEBUG_TRACE(x)
#endif


#if ENABLE_TRANSPORT_DEBUG_TRACE == 1
#define DEBUG_TRACE_TRANSPORT(x) {std::stringstream str;  TimeStamp(str) << x  ;  std::cerr << str.str() << std::endl;}
#else
/// Goal is to take these traces out in compile time to eliminate runtime cost
#define DEBUG_TRACE_TRANSPORT(x)
#endif

#if ENABLE_APPLICATION_DEBUG_TRACE == 1
#define DEBUG_TRACE_APPLICATION(x) {std::stringstream str;  TimeStamp(str) << x  ;  std::cerr << str.str() << std::endl;}
#else
/// Goal is to take these traces out in compile time to eliminate runtime cost
#define DEBUG_TRACE_APPLICATION(x)
#endif


#if ENABLE_WIRE_MSG_DEBUG_TRACE == 1
#define DEBUG_TRACE_WIRE_MSG(x) {std::stringstream str;  TimeStamp(str) << x  ;  std::cerr << str.str() << std::endl;}
#else
/// Goal is to take these traces out in compile time to eliminate runtime cost
#define DEBUG_TRACE_WIRE_MSG(x)
#endif


