#include "ExampleElement.h"
#include "ExampleComplexType_subType.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string/trim.hpp>

using namespace xercesc;
using namespace std;

int main(void) {
    XMLPlatformUtils::Initialize();

    stringstream ss, ss2;

    {
        //create ExampleElement, populate with ints and shared_ptrs to other objects of the same type
        boost::shared_ptr<ExampleElement> ect(new ExampleElement);

        ect->requiredInteger = 1;
        ect->optionalInteger = 10;

        ect->sub = boost::shared_ptr<ExampleComplexType>(new ExampleComplexType);
        ect->sub->requiredInteger = 2;

        ect->subType = boost::shared_ptr<ExampleComplexType_subType>(new ExampleComplexType_subType);
        ect->subType->integer = 2;

        for(int x = 3; x < 10; x++) {
            ect->integerArray.push_back(x + 10);
            boost::shared_ptr<ExampleComplexType> a(new ExampleComplexType);
            a->requiredInteger = x;
            ect->subArray.push_back(a);
        }

        ect->stringAttribute = "Hello world!";

        //marshal to stringstream
        ss << *ect << endl;
    }

    //print
    cout << ss.str() << endl;

    {
        boost::shared_ptr<ExampleElement> ect(new ExampleElement);

        //unmarshal to new ExampleElement
        ss >> *ect;

        //re-marshal to a string that should match the old one
        ss2 << *ect;
    }

    cout << ss2.str() << endl;

    string a = ss.str();
    string b = ss2.str();
    
    boost::algorithm::trim(a);
    boost::algorithm::trim(b);

    if(a == b)
        cout << "Success!" << endl;
    else
        cout << "String mismatch - objects probably incorrectly marshalled/unmarshalled" << endl;

    XMLPlatformUtils::Terminate();

    return 0;
}

