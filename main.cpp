/* 
 * File:   main.cpp
 * Author: tjoppen
 *
 * Created on February 12, 2010, 3:53 PM
 */

#include <iostream>
#include <vector>
#include <string>

using namespace std;

static void printUsage() {
    cout << "USAGE: james output-dir list-of-XSL-documents" << endl;
    cout << " Generates C++ classes for marshalling and unmarshalling XML to C++ objects according to the given schemas." << endl;
    cout << " Files are output in the specified output directory and are named type.h and type.cpp" << endl;
}

int main(int argc, char** argv) {
    if(argc <= 2) {
        printUsage();
        return 1;
    }

    string outputDir = argv[1];
    vector<string> schemaNames;

    for(int x = 2; x < argc; x++)
        schemaNames.push_back(argv[x]);

    return 0;
}

