/* 
 * File:   main.cpp
 * Author: tjoppen
 *
 * Created on February 12, 2010, 3:53 PM
 */

#include <iostream>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include "XercesString.h"

using namespace std;
using namespace boost;
using namespace xercesc;

static void printUsage() {
    cout << "USAGE: james output-dir list-of-XSL-documents" << endl;
    cout << " Generates C++ classes for marshalling and unmarshalling XML to C++ objects according to the given schemas." << endl;
    cout << " Files are output in the specified output directory and are named type.h and type.cpp" << endl;
}

static void work(string outputDir, const vector<string>& schemaNames) {
    XercesDOMParser parser;
    parser.setDoNamespaces(true);

    for(size_t x = 0; x < schemaNames.size(); x++) {
        string name = schemaNames[x];
        XercesString xsl("http://www.w3.org/2001/XMLSchema");

        parser.parse(name.c_str());

        DOMDocument *document = parser.getDocument();
        DOMElement *root = document->getDocumentElement();
    }
}

int main(int argc, char** argv) {
    if(argc <= 2) {
        printUsage();
        return 1;
    }

    XMLPlatformUtils::Initialize();

    string outputDir = argv[1];
    vector<string> schemaNames;

    for(int x = 2; x < argc; x++)
        schemaNames.push_back(argv[x]);

    work(outputDir, schemaNames);

    XMLPlatformUtils::Terminate();

    return 0;
}

