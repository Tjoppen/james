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

string BuiltInClass::generateElementSetter(string memberName, string nodeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tstring " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << " << memberName << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << nodeName << "->setTextContent(XercesString(" << convertedWithPostfix << "));" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

string BuiltInClass::generateAttributeSetter(string memberName, string attributeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tstring " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << " << memberName << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << attributeName << "->setValue(XercesString(" << convertedWithPostfix << "));" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

string BuiltInClass::generateParser() const {
    throw runtime_error("generateParser() called in BuiltInClass");
}

string BuiltInClass::generateMemberSetter(string memberName, string nodeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << XercesString(" << nodeName << "->getTextContent());" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << memberName << ";" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

string BuiltInClass::generateAttributeParser(string memberName, string attributeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << XercesString(" << attributeName << "->getValue());" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << memberName << ";" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

/**
 * ByteClass stuff
 */
string ByteClass::generateElementSetter(string memberName, string nodeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tstring " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << (int)" << memberName << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << convertedWithPostfix << ";" << endl;
    oss << tabs << nodeName << "->setTextContent(XercesString(" << convertedWithPostfix << "));" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

string ByteClass::generateAttributeSetter(string memberName, string attributeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tstring " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << (int)" << memberName << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << attributeName << "->setValue(XercesString(" << convertedWithPostfix << "));" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

string ByteClass::generateMemberSetter(string memberName, string nodeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tint " << tempWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << XercesString(" << nodeName << "->getTextContent());" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << tempWithPostfix << ";" << endl;
    oss << tabs << "\t" << memberName << " = " << tempWithPostfix << ";" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

string ByteClass::generateAttributeParser(string memberName, string attributeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tint " << tempWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << XercesString(" << attributeName << "->getValue());" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << tempWithPostfix << ";" << endl;
    oss << tabs << "\t" << memberName << " = " << tempWithPostfix << ";" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

/**
 * UnsignedByteClass stuff
 */
string UnsignedByteClass::generateElementSetter(string memberName, string nodeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tstring " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << (unsigned int)" << memberName << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << nodeName << "->setTextContent(XercesString(" << convertedWithPostfix << "));" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

string UnsignedByteClass::generateAttributeSetter(string memberName, string attributeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tstring " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << (unsigned int)" << memberName << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << convertedWithPostfix << ";" << endl;
    oss << tabs << "\t" << attributeName << "->setValue(XercesString(" << convertedWithPostfix << "));" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

string UnsignedByteClass::generateMemberSetter(string memberName, string nodeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tunsigned int " << tempWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << XercesString(" << nodeName << "->getTextContent());" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << tempWithPostfix << ";" << endl;
    oss << tabs << "\t" << memberName << " = " << tempWithPostfix << ";" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}

string UnsignedByteClass::generateAttributeParser(string memberName, string attributeName, string tabs) const {
    ostringstream oss;

    oss << tabs << "{" << endl;
    oss << tabs << "\tstringstream " << ssWithPostfix << ";" << endl;
    oss << tabs << "\tunsigned int " << tempWithPostfix << ";" << endl;
    oss << tabs << "\t" << ssWithPostfix << " << XercesString(" << attributeName << "->getValue());" << endl;
    oss << tabs << "\t" << ssWithPostfix << " >> " << tempWithPostfix << ";" << endl;
    oss << tabs << "\t" << memberName << " = " << tempWithPostfix << ";" << endl;
    oss << tabs << "}" << endl;

    return oss.str();
}
