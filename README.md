# CS555_PROJECT

Ray tracing using distributed computing

Directory structure:
  * raytracing/src/defines => Widely used type defines
  * raytracing/src/framework => Framework specifc defines and implementation such as messaging, threading, logging
  * raytracing/src/master => Master functionalities 
  * raytracing/src/worker => Worker related functionalities
  * raytracing/src/transport => TCP connection and send/recv functionalities 
  * raytracing/src/ray_tracer => Matt's ray tracer code
  * raytracing/src/wiremsg => All wire message definitions and implementations
  * raytracing/src/client => Client functionalities

Build instructions:
  To build code go to `raytracing` folder.
    * Code build: make
    * build clean: make clean

make is used as a front end of cmake to be able to invoke make and clean easily. So far, it produces two programs - master and worker and other build artifacts under raytracing/build. So far, master and worker are communicating with each other and a simple registration functionlity has been implemented to demonstrate basic network communication. Some of the other functionalities which are introduced so far - tcp send/recv, threading, messaging and serialization.  There is a properties file at the top level which currently have the property of the master(server name and port). 

The code architecture is mostly inspired by the observer pattern. So, there is a wide use of Listener interface between modules. All threads are designed as a single pend-point.

To kick off master(from directory raytracing):
   * ./build/master properties/master_properties.txt

To kick off worker(from directory raytracing):
   * ./build/worker properties/master_properties.txt properties/worker_properties.txt
   
To kick off client(from directory raytracing):
   * ./build/client properties/master_properties.txt "random_scene"|"cornell"  <x_dim> <y_dim> <rpp>
  
  
## Start Script
To run the start script (from directory raytracing):

`./scripts/start.sh 16 1024 768 200`

This will ssh into the first `16` machines in `scripts/275machines.txt` 
and start worker processes, then start a master on the __current__ machine,
then start a client on the __current__ machine, specifying an image of 
`1024 x 768` with `200` rays per pixel.
Because master and client are started on the current machine, change 
master_properties.txt->master_host to reflect that.
You may specify any number of machines, though a maximum of ~273 will 
be started. 

Each session/process is displayed in a tmux session, which can be 
configured to allow mouse input by running the following command 
(before starting the script):

`echo "setw -g mouse on" >> ~/.tmux.conf`

To close the session, press `Ctrl+B, :` to enter command mode (bottom left)
Then type `kill-session` and Enter. This should kill all running processes.

If you're concerned there are still running processes, just run 

`./scripts/kill.sh scripts/275machines.txt`

It will ssh into each machine in the department and kill any processes 
running under your name with `build/worker` in the command.



  


