#include "ExampleElement2.h"
#include "ExampleComplexType_subType.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <iostream>
#include <sstream>

using namespace xercesc;
using namespace std;
using namespace boost;

int main(void) {
    XMLPlatformUtils::Initialize();

    stringstream ss, ss2, ss4, ss5, ss6;
    string str3;

    {
        //create ExampleElement, populate with ints and shared_ptrs to other objects of the same type
        shared_ptr<ExampleElement2> ect(new ExampleElement2);

        ect->requiredInteger = 1;
        ect->optionalInteger = 10;
        ect->choiceA = 332;

        ect->sub = shared_ptr<ExampleComplexType>(new ExampleComplexType);
        ect->sub->requiredInteger = 2;

        ect->subType = shared_ptr<ExampleComplexType_subType>(new ExampleComplexType_subType);
        ect->subType->integer = 2;

        for(int x = 3; x < 10; x++) {
            ect->integerArray.push_back(x + 10);

            //the following demonstrates the clone cast operator
            //in other words when push_back() is called with the ExampleComplexType instance
            //the instance is cast to a shared_ptr<ExampleComplexType>
            //the cast operator makes use of ExampleComplexType::clone()
            ExampleComplexType a;
            a.requiredInteger = x;

            ect->subArray.push_back(a);
        }

        ect->stringAttribute = "Hello attribute!";
        ect->uuidAttribute = "0123456789ABCDEF0123456789ABCDEF";
        ect->intAttribute = 1337;

        ect->extensionInt = 15;
        ect->extensionAttribute = "Hello extension!";

        //marshal to stringstream
        ss << *ect;
        ss2 << *ect;
        str3 = *ect;
    }

    //print
    cout << ss.str() << endl;

    //unmarshal using blank ExampleElement2
    {
        //create new ExampleElement2
        shared_ptr<ExampleElement2> ect(new ExampleElement2);

        //unmarshal
        ss >> *ect;

        //re-marshal to a string that should match the old one
        ss4 << *ect;
    }

    //unmarshal using istream constructor
    {
        shared_ptr<ExampleElement2> ect(new ExampleElement2(ss2));
        ss5 << *ect;
    }

    //unmarshal using string constructor
    {
        shared_ptr<ExampleElement2> ect(new ExampleElement2(str3));
        ss6 << *ect;
    }

    cout << "From blank instance: " << ss4.str() << endl;
    cout << "Via istream constructor: " << ss5.str() << endl;
    cout << "Via string constructor: " << ss6.str() << endl;

    if(ss.str() == ss4.str() && ss2.str() == ss5.str() && str3 == ss6.str())
        cout << "Success!" << endl;
    else
        cout << "String mismatch - objects probably incorrectly marshalled/unmarshalled" << endl;

    XMLPlatformUtils::Terminate();

    return 0;
}

