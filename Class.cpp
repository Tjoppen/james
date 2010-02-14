/* 
 * File:   Class.cpp
 * Author: tjoppen
 * 
 * Created on February 14, 2010, 4:20 PM
 */

#include "Class.h"
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace std;

Class::Class(FullName name, ClassType type) : name(name), type(type), base(NULL) {
}

Class::Class(FullName name, ClassType type, Class *base) : name(name),
        type(type), base(base) {
}

Class::~Class() {
}

void Class::addMember(string name, Member memberInfo) {
    if(members.find(name) != members.end())
        throw runtime_error("Member " + name + " defined more than once in " + this->name.second);

    cout << this->name.second << " got " << memberInfo.type.first << ":" << memberInfo.type.second << " " << name << ". Occurance: ";

    if(memberInfo.maxOccurs == UNBOUNDED) {
        cout << "at least " << memberInfo.minOccurs;
    } else if(memberInfo.minOccurs == memberInfo.maxOccurs) {
        cout << "exactly " << memberInfo.minOccurs;
    } else {
        cout << "between " << memberInfo.minOccurs << "-" << memberInfo.maxOccurs;
    }

    cout << endl;

    members[name] = memberInfo;
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
    return "";
}

string Class::generateMemberSetter(string memberName, string nodeName) const {
    return memberName + "->parseNode(" + nodeName + ");";
}

string Class::getClassname() const {
    return "shared_ptr<" + name.second + ">";
}
