all:
	mkdir -p generated
	james generated example.xsd
	g++ example.cpp generated/*.cpp -ljames -lxerces-c -o example
