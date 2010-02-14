/* 
 * File:   Class.cpp
 * Author: tjoppen
 * 
 * Created on February 14, 2010, 4:20 PM
 */

#include "Class.h"
#include <sstream>

using namespace std;

Class::Class(FullName name, ClassType type) : name(name), type(type), base(NULL) {
}

Class::Class(FullName name, ClassType type, Class *base) : name(name),
        type(type), base(base) {
}

Class::~Class() {
}

/**
 * Default implementation of generateAppender()
 */
string Class::generateAppender() const {
    ostringstream oss;
    
    for(std::map<std::string, Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        string name = it->first;
        string nodeName = name + "Node";

        oss << "DOMElement *" << nodeName << " = node->getOwnerDocument()->createElement(X(\"" << name << "\"));" << endl;
        oss << it->second.cl->generateNodeSetter(name, nodeName) << endl;
    }

    return oss.str();
}

string Class::generateNodeSetter(string memberName, string nodeName) const {
    return memberName + "->appendChildren(" + nodeName + ");";
}

string Class::generateParser() const {

}

string Class::getClassname() const {
    return "shared_ptr<" + name.second + ">";
}
