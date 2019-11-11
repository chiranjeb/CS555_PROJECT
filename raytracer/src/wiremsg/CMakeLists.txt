list(APPEND RAY_TRACER_RENDERER_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/wire_msg.cpp
	${CMAKE_CURRENT_LIST_DIR}/wire_msg_factory.cpp
	${CMAKE_CURRENT_LIST_DIR}/worker_registration_msg.cpp
)

list(APPEND RAY_TRACER_WORKER_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/wire_msg.cpp
	${CMAKE_CURRENT_LIST_DIR}/wire_msg_factory.cpp
	${CMAKE_CURRENT_LIST_DIR}/worker_registration_msg.cpp

)


