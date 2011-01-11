cmake_minimum_required(VERSION 2.6)
project(james)

add_subdirectory(libjames)

add_executable(james
	main.cpp
	Class.cpp
	BuiltInClasses.cpp
)

target_link_libraries(james libjames xerces-c)
install(TARGETS james DESTINATION bin)

