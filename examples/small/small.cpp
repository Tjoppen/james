#include "ExampleElement.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <iostream>

using namespace xercesc;
using namespace std;

int main(void) {
    XMLPlatformUtils::Initialize();

    {
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

        cout << *ect << endl;
    }

    XMLPlatformUtils::Terminate();

    return 0;
}

