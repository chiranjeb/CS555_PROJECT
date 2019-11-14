#include <iostream>
#include <array>
#include <streambuf>
#include<cstring>
#include<cstdio>
#include<sstream>

int main()
{
   PreAllocatedStreamBuffer<100> stream_buffer;

   //std::ostringstream str(&stream_buffer);
   //std::basic_iostream<char> str(&stream_buffer);
   //std::ostream str(&stream_buffer);
   std::stringstream str; 
   int origSize = str.tellp();
   std::cerr << "Buffer length" << "previous:" << origSize << "now:" << stream_buffer.Tellp() << std::endl;
   int i=5, j=1, k=2;
   char ch1='c';
   std::string s1, s2, s3, s4;
   s1 = std::string("hello");
   s2 = std::string("prachi") ;
   s3 = std::string("pogi");
   str << i << " ";
   str << s1 << " ";
   str << j << " ";
   str << s2 << " ";
   str << k << " ";
   str << s3 << " ";
   str << ch1 << " ";

   std::cerr << "Buffer length" << "previous:" << origSize << "now:" << stream_buffer.Tellp() << std::endl;

   if( str.good())
    std::cerr << "stream is good:" << std::endl;


   PreAllocatedStreamBuffer<100> stream_buffer2;
   std::memcpy(&stream_buffer2.m_buffer[0], &stream_buffer.m_buffer[0], stream_buffer.Tellp());
    

   stream_buffer2.Setg(stream_buffer.Tellp());
   int i1, j1, k1;
   char ch2;
   std::string s11, s22, s33, s44;
   //std::istream istr(&stream_buffer2);

   std::stringstream istr(str.str()); 
   istr >> i1 >> s11>> j1 >> s22>> k1 >> s33 >> ch2;
   //std::cerr << s11 << "\n" << s22 << "\n" << "\n" << i1 << "\n" << s33 << std::endl;
   std::cerr << s11 << "\n" << s22  << "\n" << s33 << std::endl;

   std::cerr << i1 << " " << j1 << " " << k1 << ch2 << std::endl;

}
