#include "ExampleElement.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <iostream>
#include <sstream>

using namespace xercesc;
using namespace std;

int main(void) {
    XMLPlatformUtils::Initialize();

    stringstream ss;

    {
        //create ExampleElement, populate with ints and shared_ptrs to other objects of the same type
        boost::shared_ptr<ExampleElement> ect(new ExampleElement);

        ect->requiredInteger = 1;
        ect->optionalInteger = 10;

        ect->sub = boost::shared_ptr<ExampleComplexType>(new ExampleComplexType);
        ect->sub->requiredInteger = 2;

        for(int x = 3; x < 10; x++) {
            ect->integerArray.push_back(x + 10);
            boost::shared_ptr<ExampleComplexType> a(new ExampleComplexType);
            a->requiredInteger = x;
            ect->subArray.push_back(a);
        }

        //marshal to stringstream
        ss << *ect << endl;
    }

    //print
    cout << ss.str();

    {
        boost::shared_ptr<ExampleElement> ect(new ExampleElement);

        //unmarshal to new ExampleElement
        ss >> *ect;
    }

    XMLPlatformUtils::Terminate();

    return 0;
}

