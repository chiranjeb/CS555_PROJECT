#include "properties_reader.hpp"

int main ()
{
   std::string filename = "renderer_properties.txt";
   PropertiesReader properties(filename);
   std::string hostname = properties["renderer_host"];
   std::cerr << "Renderer host:" << hostname << std::endl;
   std::cerr << "Renderer port:" << properties["renderer_listening_port"] << std::endl;
}
