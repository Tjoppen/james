/* 
 * File:   Class.cpp
 * Author: tjoppen
 * 
 * Created on February 14, 2010, 4:20 PM
 */

#include "Class.h"

using namespace std;

Class::Class(FullName name, ClassType type) : name(name), type(type), base(NULL) {
}

Class::Class(FullName name, ClassType type, Class *base) : name(name),
        type(type), base(base) {
}

Class::~Class() {
}

