add_library(libjames
    Exceptions.cpp
    HexBinary.cpp
    XercesString.cpp
    XMLDocument.cpp
    XMLObject.cpp
)

set_target_properties(libjames PROPERTIES OUTPUT_NAME james)
install(TARGETS libjames DESTINATION lib)

install(FILES
    Exceptions.h
    HexBinary.h
    version.h
    XercesString.h
    XMLDocument.h
    XMLObject.h
    optional.h
    DESTINATION include/libjames
)

