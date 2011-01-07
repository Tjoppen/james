#include "ExampleElement2.h"
#include "ExampleComplexType_subType.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <iostream>
#include <sstream>

using namespace xercesc;
using namespace std;

int main(void) {
    XMLPlatformUtils::Initialize();

    stringstream ss, ss2, ss4, ss5, ss6;
    string str3;

    {
        //create ExampleElement, populate with ints and other objects of the same type
        ExampleElement2 ect;

        ect.requiredInteger = 1;
        ect.optionalInteger = 10;
        ect.choiceA = 332;

        ect.sub = ExampleComplexType();
        ect.sub->requiredInteger = 2;

        ect.subType = ExampleComplexType_subType();
        ect.subType->integer = 2;

        for(int x = 3; x < 10; x++) {
            ect.integerArray.push_back(x + 10);

            ExampleComplexType a;
            a.requiredInteger = x;

            ect.subArray.push_back(a);
        }

        ect.stringAttribute = "Hello attribute!";
        ect.uuidAttribute = "0123456789ABCDEF0123456789ABCDEF";
        ect.intAttribute = 1337;

        ect.extensionInt = 15;
        ect.extensionAttribute = "Hello extension!";
        ect.hex = "Hello hex!";

        //marshal to stringstream
        ss << ect;
        ss2 << ect;
        str3 = ect;
    }

    //print
    cout << ss.str() << endl;

    //unmarshal using blank ExampleElement2
    {
        //create new ExampleElement2
        ExampleElement2 ect;

        //unmarshal
        ss >> ect;

        //re-marshal to a string that should match the old one
        ss4 << ect;
    }

    //unmarshal using istream constructor
    {
        ExampleElement2 ect(ss2);
        ss5 << ect;
    }

    //unmarshal using string constructor
    {
        ExampleElement2 ect = ExampleElement2::fromString(str3);
        ss6 << ect;
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

