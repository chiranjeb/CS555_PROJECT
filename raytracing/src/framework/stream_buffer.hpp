#include <iostream>
#include <array>
#include <streambuf>

// Buffer for std::ostream implemented by std::array
template<std::size_t SIZE_IN_BYTES>
class PreAllocatedStreamArrayBuffer : public std::streambuf
{
public:
   // PreAllocatedStreamArrayBuffer
   PreAllocatedStreamArrayBuffer()
   {
      // set std::basic_streambuf
      std::streambuf::setp(m_buffer.begin(), m_buffer.end());
   }

   // Find the size of the buffer
   long Tellp()
   {
      return int(std::streambuf::pptr() - &m_buffer[0]);
   }

private:
   std::array<char, SIZE_IN_BYTES> m_buffer;
};


class PreAllocatedStreamBuffer : public std::streambuf
{
public:
   // PreAllocatedStreamBuffer
   PreAllocatedStreamBuffer(char *buffer, int size):m_buffer(buffer), m_size(size)
   {
      // set std::basic_streambuf
      std::streambuf::setp(buffer, buffer+size);
   }

   // Find the size of the buffer
   long Tellp()
   {
      return int(std::streambuf::pptr() - m_buffer);
   }

   void Setg(int index)
   {
      setg(m_buffer,m_buffer, m_buffer+index);
   }

private:
   char *m_buffer;
   int m_size;
};
