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
#include <boost/shared_ptr.hpp>

using namespace std;

Class::Class(FullName name, ClassType type) : name(name), type(type), 
        base(NULL), isBasic(false), isDocument(false), hasBase(false) {
}

Class::Class(FullName name, ClassType type, FullName baseType) : name(name),
        type(type), base(NULL), isBasic(false), isDocument(false),
        hasBase(true), baseType(baseType) {
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
        string setterName = it->first;
        string nodeName = name + "Node";

        if(it->second.isArray()) {
            setterName = "(*it)";
            oss << "for(" << it->second.getType() << "::const_iterator it = " << name << ".begin(); it != " << name << ".end(); it++) {" << endl;
        } else if(!it->second.isRequired()) {
            //insert a non-null check
            oss << "if(" << name << ") {" << endl;
        } else {
            //required member - check for its existance
        }

        oss << "DOMElement *" << nodeName << " = node->getOwnerDocument()->createElement(X(\"" << name << "\"));" << endl;
        oss << it->second.cl->generateNodeSetter(setterName, nodeName) << endl;
        oss << "node->appendChild(" << nodeName << ");" << endl;

        if(it->second.isArray() || !it->second.isRequired())
            oss << "}" << endl;

        oss << endl;
    }

    return oss.str();
}
//systemet?
string Class::generateNodeSetter(string memberName, string nodeName) const {
    return memberName + "->appendChildren(" + nodeName + ");";
}

string Class::generateParser() const {
    ostringstream oss;

    oss << "for(DOMNode *child = node->getFirstChild(); child; child = child->getNextSibling()) {" << endl;
    oss << "if(!child->getLocalName()) continue;" << endl;
    oss << "XercesString name(child->getLocalName());" << endl;

    //TODO: replace this with a map<pair<string, DOMNode::ElementType>, void(*)(DOMNode*)> thing?
    //in other words, lookin up parsing function pointers in a map should be faster then all these string comparisons
    for(std::map<std::string, Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        oss << "if(name == \"" << it->first << "\" && child->getNodeType() == DOMNode::" << (it->second.isAttribute ? "ATTRIBUTE_NODE" : "ELEMENT_NODE") << ") {" << endl;

        string memberName = it->first;
        if(it->second.isArray()) {
            memberName = "temp";
            oss << it->second.cl->getClassType() << " " << memberName;

            if(it->second.cl->isBasic) {
                //for basic types we're done
                oss << ";" << endl;
            } else {
                //for non-basic types we need to set the contents of the shared_ptr
                oss << "(new " << it->second.cl->getClassname() << ");" << endl;
            }
        } else if(!it->second.cl->isBasic) {
            //add check and allocation of shared_ptr
            oss << "if(!" << memberName << ") " << memberName << " = boost::shared_ptr<" << it->second.cl->getClassname() << ">(new " << it->second.cl->getClassname() << ");" << endl;
        }

        oss << it->second.cl->generateMemberSetter(memberName, "child");

        if(it->second.isArray()) {
            oss << it->first << ".push_back(" << memberName << ");" << endl;
        }

        oss << "}" << endl;
    }

    oss << "}" << endl;

    return oss.str();
}

string Class::generateMemberSetter(string memberName, string nodeName) const {
    ostringstream oss;

    //TODO: add check & allocation for shared_ptr if non-basic
    
    oss << memberName << "->parseNode(" << nodeName << ");" << endl;

    return oss.str();
}

string Class::getClassname() const {
    return name.second;
}

string Class::getClassType() const {
    if(isBasic)
        return getClassname();
    else
        return "boost::shared_ptr<" + name.second + ">";
}

string Class::getBaseClassname() const {
    return hasBase ? base->getClassname() : "james::XMLObject";
}

string Class::getBaseHeader() const {
    return hasBase ? base->getClassname() + ".h" : "XMLObject.h";
}

void Class::writeImplementation(ostream& os) const {
    //cout << "writeImplementation()" << endl;
    ClassName className = name.second;

    os << "#include <sstream>" << endl;
    os << "#include <xercesc/dom/DOMDocument.hpp>" << endl;
    os << "#include <xercesc/dom/DOMElement.hpp>" << endl;
    os << "#include \"XercesString.h\"" << endl;
    os << "#include \"" << className << ".h\"" << endl;

    //include headers of all non-basic member types that aren't us
    for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(!it->second.cl->isBasic && it->second.cl != this)
            os << "#include \"" << it->second.cl->name.second << ".h\"" << endl;

    os << "using namespace std;" << endl;
    os << "using namespace xercesc;" << endl;

    //constructor
    os << className << "::" << className << "() : " << getBaseClassname() << "() {";
    
    //give all basic optional members a default value of zero
    for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(!it->second.isArray() && !it->second.isRequired() && it->second.cl->isBasic)
            os << it->first << " = 0;" << endl;

    os << "}" << endl << endl;

    //method implementation
    if(isDocument) {
        os << "std::string " << className << "::getName() const {" << endl;
        os << "return \"" << className << "\";" << endl;
        os << "}" << endl;
    } else {
        os << "void " << className << "::appendChildren(xercesc::DOMNode *node) const {" << endl;
        
        //call base appender
        if(hasBase)
            os << base->name.second << "::appendChildren(node);" << endl;

        os << generateAppender() << endl;
        os << "}" << endl << endl;

        os << "void " << className << "::parseNode(xercesc::DOMNode *node) {" << endl;
        os << generateParser() << endl;
        os << "}" << endl << endl;
    }
}

void Class::writeHeader(ostream& os) const {
    ClassName className = name.second;

    os << "#ifndef _" << className << "_H" << endl;
    os << "#define _" << className << "_H" << endl;

    os << "#include <vector>" << endl;
    os << "#include <boost/shared_ptr.hpp>" << endl;
    os << "#include \"" << getBaseHeader() << "\"" << endl;

    if(isDocument)
        os << "#include \"XMLDocument.h\"" << endl;

    //non-basic member class prototypes
    for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(!it->second.cl->isBasic)
            os << "class " << it->second.cl->getClassname() << ";" << endl;

    os << "class " << className << " : public " << getBaseClassname();
    
    if(isDocument)
        os << ", public james::XMLDocument" << endl;

    os << " {" << endl;
    os << "public:" << endl;

    os << className << "();" << endl;

    //prototypes
    if(isDocument)
        os << "std::string getName() const;" << endl;
    else {
        os << "void appendChildren(xercesc::DOMNode *node) const;" << endl;
        os << "void parseNode(xercesc::DOMNode *node);" << endl;
    }
    
    //members
    for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        os << it->second.getType() << " " << it->first << ";" << endl;
    }

    os << "};" << endl;

    os << "#endif //_" << className << "_H" << endl;
}

bool Class::Member::isArray() const {
    return maxOccurs > 1 || maxOccurs == UNBOUNDED;
}

bool Class::Member::isRequired() const {
    return minOccurs >= 1;
}

string Class::Member::getType() const {
    //vector if array, regular member otherwise
    if(isArray()) {
        return "std::vector<" + cl->getClassType() + " >";
    } else
        return cl->getClassType();
}
