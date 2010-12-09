cmake_minimum_required(VERSION 2.6)
project(james)

include_directories(libjames)

add_executable(james
	main.cpp
	Class.cpp
	BuiltInClasses.cpp
	libjames/HexBinary.cpp
	libjames/XercesString.cpp
	libjames/XMLDocument.cpp
	libjames/XMLObject.cpp
	)

target_link_libraries(james xerces-c)

