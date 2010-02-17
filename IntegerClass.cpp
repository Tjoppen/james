/* 
 * File:   IntegerClass.cpp
 * Author: tjoppen
 * 
 * Created on February 14, 2010, 6:48 PM
 */

#include <stdexcept>
#include <sstream>
#include "IntegerClass.h"
#include "main.h"

using namespace std;

IntegerClass::IntegerClass() : Class(FullName(XSL, "int"), Class::SIMPLE_TYPE) {
}

IntegerClass::~IntegerClass() {
}

string IntegerClass::generateAppender() const {
    throw runtime_error("generateAppender() called in IntegerClass");
}

string IntegerClass::generateNodeSetter(string memberName, string nodeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "string converted;" << endl;
    oss << "ss << " << memberName << ";" << endl;
    oss << "ss >> converted;" << endl;
    oss << nodeName << "->setTextContent(XercesString(converted));" << endl;
    oss << "}" << endl;

    return oss.str();
}

string IntegerClass::generateParser() const {
    throw runtime_error("generateParser() called in IntegerClass");
}

string IntegerClass::generateMemberSetter(string memberName, string nodeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "ss << XercesString(" << nodeName << "->getTextContent());" << endl;
    oss << "ss >> " << memberName << ";" << endl;
    oss << "}" << endl;

    return oss.str();
}

string IntegerClass::getClassname() const {
    return "int";
}
