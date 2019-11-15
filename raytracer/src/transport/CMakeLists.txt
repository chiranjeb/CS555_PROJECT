list ( APPEND RAY_TRACER_RENDERER_SOURCES 
		${CMAKE_CURRENT_LIST_DIR}/transport_mgr.cpp
		${CMAKE_CURRENT_LIST_DIR}/tcp_io_server.cpp
		${CMAKE_CURRENT_LIST_DIR}/tcp_io_send_recv.cpp
		${CMAKE_CURRENT_LIST_DIR}/tcp_io_connection.cpp

	)
	
list ( APPEND RAY_TRACER_WORKER_SOURCES 
		${CMAKE_CURRENT_LIST_DIR}/tcp_io_server.cpp
		${CMAKE_CURRENT_LIST_DIR}/tcp_io_connection.cpp
		${CMAKE_CURRENT_LIST_DIR}/transport_mgr.cpp
		${CMAKE_CURRENT_LIST_DIR}/tcp_io_send_recv.cpp
	)
	

