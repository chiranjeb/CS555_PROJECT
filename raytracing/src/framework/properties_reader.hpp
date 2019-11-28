#include <iostream>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <bits/stdc++.h>


class PropertiesReader
{
public:
   PropertiesReader(std::string &filename)
   {
      std::ifstream file(filename);
      std::string line;
      while (std::getline(file, line))
      {
         /// Let's filter out the lines with the comments.
         if ( line.find("=") != std::string::npos)
         {
         std::stringstream linestream(line);
         std::string token1, token2;
         std::getline(linestream, token1, '=');
         std::getline(linestream, token2, '=');
         m_PropertiesVsValue[token1] = token2;
         }
      }
   }

   std::string operator[](std::string key)
   {
      return m_PropertiesVsValue[key];
   }


protected:
   std::map<std::string, std::string> m_PropertiesVsValue;
};


