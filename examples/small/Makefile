all:
	if test ! -d generated; then mkdir generated; fi
	../../dist/Debug/GNU-Linux-x86/james generated/ small.xsd
	g++ small.cpp generated/*.cpp ../../XML*.cpp ../../XercesString.cpp ../../HexBinary.cpp -I../../ -Igenerated -lxerces-c -g -o small

