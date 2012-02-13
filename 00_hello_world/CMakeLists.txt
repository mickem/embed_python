cmake_minimum_required(VERSION 2.6)

PROJECT(00_hello_world)


SET(SOURCE_FILES
	hello_world.cpp
)

IF(WIN32)
	SET(SOURCE_FILES ${SOURCE_FILES}
		hello_world.hpp
	)
ENDIF(WIN32)

add_executable(hello_world ${SOURCE_FILES})
target_link_libraries(hello_world)
