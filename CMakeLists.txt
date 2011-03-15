cmake_minimum_required(VERSION 2.6)
project(james)

add_subdirectory(libjames)

add_executable(james
	main.cpp
	Class.cpp
	BuiltInClasses.cpp
)

target_link_libraries(james libjames)

# Account for the Xerces-C++ 3.1.x library being called xerces-c_3 on WIN32
if(WIN32)
	target_link_libraries(james xerces-c_3)
else(WIN32)
	target_link_libraries(james xerces-c)
endif(WIN32)

install(TARGETS james DESTINATION bin)

