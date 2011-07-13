#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
#include "generated/PersonDocument.h"
#include "generated/PersonListDocument.h"

int main() {
    //initialize Xerces-C++
    xercesc::XMLPlatformUtils::Initialize();

    //unmarshal personIn from stdin
    PersonDocument personIn(std::cin);

    //create a list containing personIn and some other person
    PersonListDocument list;
    list.person.push_back(personIn);
    list.person.push_back(PersonType("Some Otherguy", "Somewhere 999", 1985));

    //finally, marshal the list to stdout
    std::cout << list;

    //terminate Xerces-C++
    xercesc::XMLPlatformUtils::Terminate();

    return 0;
}
