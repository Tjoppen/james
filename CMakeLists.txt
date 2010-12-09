cmake_minimum_required(VERSION 2.6)
project(james)

add_subdirectory(libjames)

add_executable(james-bin
	main.cpp
	Class.cpp
	BuiltInClasses.cpp
	)

target_link_libraries(james-bin james xerces-c)
set_target_properties(james-bin PROPERTIES OUTPUT_NAME james)
install(TARGETS james-bin DESTINATION bin)

