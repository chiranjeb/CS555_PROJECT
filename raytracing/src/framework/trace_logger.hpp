#pragma once
#include <iostream>
#include <time.h>
#include <iomanip>
#include <sstream>

/// Set this to 1 to turn off all the DEBUG STREAMS.
#define RELEASE_BUILD 0

/// Get a timestamp
extern std::ostream& TimeStamp(std::ostream& ostr);

/// TODO: consider changing std::cerr to a fstream to output to log
#define RELEASE_TRACE(x) {std::stringstream str;  TimeStamp(str) << x  ;  std::cerr << str.str() << std::endl;}
 
#if RELEASE_BUILD == 0
//#define DEBUG_TRACE(x) TimeStamp(std::cerr) << x << std::endl
#define DEBUG_TRACE(x) {std::stringstream str;  TimeStamp(str) << x  ;  std::cerr << str.str() << std::endl;}
#else
/// Goal is to take these traces out in compile time to eliminate runtime cost
#define DEBUG_TRACE(x)
#endif