list(APPEND RAY_TRACER_MASTER_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/master_main.cpp
	${CMAKE_CURRENT_LIST_DIR}/master_scheduler.cpp
	${CMAKE_CURRENT_LIST_DIR}/scheduler_base.cpp
	${CMAKE_CURRENT_LIST_DIR}/scene_scheduler_static.cpp
	${CMAKE_CURRENT_LIST_DIR}/scene_scheduler_dynamic.cpp
	${CMAKE_CURRENT_LIST_DIR}/resource_tracker.cpp
)

