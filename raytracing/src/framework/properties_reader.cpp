#include "properties_reader.hpp"

int main ()
{
   std::string filename = "master_properties.txt";
   PropertiesReader properties(filename);
   std::string hostname = properties["master_host"];
   std::cerr << "Master host:" << hostname << std::endl;
   std::cerr << "Master port:" << properties["master_listening_port"] << std::endl;
}
