list(APPEND RAY_TRACER_MASTER_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/messaging_framework.cpp
	${CMAKE_CURRENT_LIST_DIR}/trace_logger.cpp
)

list(APPEND RAY_TRACER_WORKER_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/messaging_framework.cpp
	${CMAKE_CURRENT_LIST_DIR}/trace_logger.cpp
)

list(APPEND RAY_TRACER_CLIENT_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/messaging_framework.cpp
	${CMAKE_CURRENT_LIST_DIR}/trace_logger.cpp
)



