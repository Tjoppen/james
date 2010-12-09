add_library(james
	HexBinary.cpp
	XercesString.cpp
	XMLDocument.cpp
	XMLObject.cpp)

install(TARGETS james DESTINATION lib)

install(FILES
	HexBinary.h
	XercesString.h
	XMLDocument.h
	XMLObject.h
	DESTINATION include/libjames)

