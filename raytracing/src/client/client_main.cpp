#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "client.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc == 7)
    {
       /// Read master server address
       std::string master_properties_file = argv[1];
       PropertiesReader  master_properties(master_properties_file);
       std::string master_address = master_properties["master_host"];
       int master_port = std::stoi(master_properties["master_listening_port"]);
       RELEASE_TRACE("Master server and port address: " << master_address << "," << master_port);

       std::string worker_properties_file_name = argv[2];
       PropertiesReader  client_properties(worker_properties_file_name);
       int client_thread_q_depth = std::stoi(client_properties["client_thread_q_depth"]);
       int client_server_listening_q_depth = std::stoi(client_properties["client_server_listening_q_depth"]);

       /// Instantiate and start the client
        std::string scene_name = argv[3];
        std::uint32_t width = stoi(argv[4]);
        std::uint32_t height = stoi(argv[5]);
        std::uint32_t rpp = stoi(argv[6]);

       RELEASE_TRACE("[Client Properties] Client server listening queue depth: " << client_server_listening_q_depth);
       RELEASE_TRACE("[Client Properties] Client threadQ depth: " << client_thread_q_depth);
       RELEASE_TRACE("Producing scene: " << scene_name);
       Client::Instance().Instantiate(master_address, master_port, client_thread_q_depth, scene_name, width, height, rpp);

       /// Start the client server where the pixels will be sent to
       TransportMgr::Instance().CreateTCPServer(0, client_server_listening_q_depth, Client::Instance().GetThrdListener());

       /// Put the main thread to sleep.
       while (1)
       {
          std::this_thread::sleep_for(std::chrono::milliseconds(60000));
       }
    }
    else
    {
        RELEASE_TRACE("Usage: ./build/client properties/master_properties.txt properties/client_properties.txt <scene_name> ");
    }
}

