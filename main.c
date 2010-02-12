/* 
 * File:   main.c
 * Author: tjoppen
 *
 * Created on February 12, 2010, 3:53 PM
 */

#include <stdio.h>
#include <stdlib.h>

static void printUsage() {
    printf("USAGE: james output-dir list-of-XSL-documents\n");
    printf(" Generates C++ classes for marshalling and unmarshalling XML to C++ objects according to the given schemas.\n");
    printf(" Files are output in the specified output directory and are named type.h and type.cpp\n");
}

int main(int argc, char** argv) {
    if(argc <= 2) {
        printUsage();
        return 1;
    }

    return 0;
}

