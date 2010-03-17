/* 
 * File:   BuiltInClasses.cpp
 * Author: tjoppen
 * 
 * Created on February 14, 2010, 6:48 PM
 */

#include <stdexcept>
#include <sstream>
#include "BuiltInClasses.h"
#include "main.h"

using namespace std;

BuiltInClass::BuiltInClass(string name) : Class(FullName(XSL, name), Class::SIMPLE_TYPE) {
}

BuiltInClass::~BuiltInClass() {
}

bool BuiltInClass::isBuiltIn() const {
    return true;
}

string BuiltInClass::generateAppender() const {
    throw runtime_error("generateAppender() called in BuiltInClass");
}

string BuiltInClass::generateElementSetter(string memberName, string nodeName) const {
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

string BuiltInClass::generateAttributeSetter(string memberName, string attributeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "string converted;" << endl;
    oss << "ss << " << memberName << ";" << endl;
    oss << "ss >> converted;" << endl;
    oss << attributeName << "->setValue(XercesString(converted));" << endl;
    oss << "}" << endl;

    return oss.str();
}

string BuiltInClass::generateParser() const {
    throw runtime_error("generateParser() called in BuiltInClass");
}

string BuiltInClass::generateMemberSetter(string memberName, string nodeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "ss << XercesString(" << nodeName << "->getTextContent());" << endl;
    oss << "ss >> " << memberName << ";" << endl;
    oss << "}" << endl;

    return oss.str();
}

string BuiltInClass::generateAttributeParser(string memberName, string attributeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "ss << XercesString(" << attributeName << "->getValue());" << endl;
    oss << "ss >> " << memberName << ";" << endl;
    oss << "}" << endl;

    return oss.str();
}

/**
 * ByteClass stuff
 */
string ByteClass::generateElementSetter(string memberName, string nodeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "string converted;" << endl;
    oss << "ss << (int)" << memberName << ";" << endl;
    oss << "ss >> converted;" << endl;
    oss << nodeName << "->setTextContent(XercesString(converted));" << endl;
    oss << "}" << endl;

    return oss.str();
}

string ByteClass::generateAttributeSetter(string memberName, string attributeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "string converted;" << endl;
    oss << "ss << (int)" << memberName << ";" << endl;
    oss << "ss >> converted;" << endl;
    oss << attributeName << "->setValue(XercesString(converted));" << endl;
    oss << "}" << endl;

    return oss.str();
}

string ByteClass::generateMemberSetter(string memberName, string nodeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "int temp;" << endl;
    oss << "ss << XercesString(" << nodeName << "->getTextContent());" << endl;
    oss << "ss >> temp;" << endl;
    oss << memberName << " = temp;" << endl;
    oss << "}" << endl;

    return oss.str();
}

string ByteClass::generateAttributeParser(string memberName, string attributeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "int temp;" << endl;
    oss << "ss << XercesString(" << attributeName << "->getValue());" << endl;
    oss << "ss >> temp;" << endl;
    oss << memberName << " = temp;" << endl;
    oss << "}" << endl;

    return oss.str();
}

/**
 * UnsignedByteClass stuff
 */
string UnsignedByteClass::generateElementSetter(string memberName, string nodeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "string converted;" << endl;
    oss << "ss << (unsigned int)" << memberName << ";" << endl;
    oss << "ss >> converted;" << endl;
    oss << nodeName << "->setTextContent(XercesString(converted));" << endl;
    oss << "}" << endl;

    return oss.str();
}

string UnsignedByteClass::generateAttributeSetter(string memberName, string attributeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "string converted;" << endl;
    oss << "ss << (unsigned int)" << memberName << ";" << endl;
    oss << "ss >> converted;" << endl;
    oss << attributeName << "->setValue(XercesString(converted));" << endl;
    oss << "}" << endl;

    return oss.str();
}

string UnsignedByteClass::generateMemberSetter(string memberName, string nodeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "unsigned int temp;" << endl;
    oss << "ss << XercesString(" << nodeName << "->getTextContent());" << endl;
    oss << "ss >> temp;" << endl;
    oss << memberName << " = temp;" << endl;
    oss << "}" << endl;

    return oss.str();
}

string UnsignedByteClass::generateAttributeParser(string memberName, string attributeName) const {
    ostringstream oss;

    oss << "{" << endl;
    oss << "stringstream ss;" << endl;
    oss << "unsigned int temp;" << endl;
    oss << "ss << XercesString(" << attributeName << "->getValue());" << endl;
    oss << "ss >> temp;" << endl;
    oss << memberName << " = temp;" << endl;
    oss << "}" << endl;

    return oss.str();
}
