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
   * ./build/client properties/master_properties.txt "random_scene"
  



  


