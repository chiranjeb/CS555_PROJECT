#include "trace_logger.hpp"

/// Output a timestamp
std::ostream& TimeStamp(std::ostream& ostr)
{
   // Get the localtime
   time_t time_ptr = time(NULL);
   tm *tm_local = localtime(&time_ptr);
   ostr << std::setfill('0') << std::setw(2) << tm_local->tm_hour << ":" << std::setw(2) << tm_local->tm_min << ":" << std::setw(2) << tm_local->tm_sec << "|";
   return ostr;
}

#if UNIT_TEST
int main()
{
   RELEASE_TRACE("Hello");
   RELEASE_TRACE("Hello" << " test");
   DEBUG_TRACE("Hello");
}
#endif