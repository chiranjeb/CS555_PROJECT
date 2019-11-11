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

private:
   char *m_buffer;
   int m_size;
};


/*
int main()
{
   PreAllocatedStreamBuffer<100> stream_buffer;

   //std::ostringstream str(&stream_buffer);
   //std::basic_iostream<char> str(&stream_buffer);
   std::ostream str(&stream_buffer);
   int origSize = str.tellp();
   std::cerr << "Buffer length" << "previous:" << origSize << "now:" << stream_buffer.Tellp() << std::endl;
   int i=5;
   std::string s1, s2, s3, s4;
   s1 = "hello";
   s2 = "prachi" ;
   s3 = "pogi";
   str << i;
   str << s1;
   str << s2;
   str << s3;
   std::cerr << "Buffer length" << "previous:" << origSize << "now:" << stream_buffer.Tellp() << std::endl;

   if( str.good())
    std::cerr << "stream is good:" << std::endl;

   std::istream istr(&stream_buffer);
   istr >> i >> s1 >> s2 >> s3;
   std::cerr << s1 << " " << s2 << " " << " " << i << " " << s3 << std::endl;
}
*/
