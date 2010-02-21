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
        base(NULL), isDocument(false), hasBase(false) {
}

Class::Class(FullName name, ClassType type, FullName baseType) : name(name),
        type(type), base(NULL), isDocument(false),
        hasBase(true), baseType(baseType) {
}

Class::~Class() {
}

bool Class::isSimple() const {
    return type == SIMPLE_TYPE;
}

bool Class::isBuiltIn() const {
    return false;
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
            oss << "if(" << it->second.cl->getTester(name) << ") {" << endl;
        } else {
            //required member - check for its existance
        }

        if(it->second.isAttribute) {
            //attribute
            //TODO: correct implementation
            if(!it->second.cl->isSimple())
                throw runtime_error("Tried to generate non-simple attribute " + it->first + " in " + this->name.second);
        } else {
            oss << "DOMElement *" << nodeName << " = node->getOwnerDocument()->createElement(X(\"" << name << "\"));" << endl;
            oss << it->second.cl->generateNodeSetter(setterName, nodeName) << endl;
            oss << "node->appendChild(" << nodeName << ");" << endl;
        }

        if(it->second.isArray() || !it->second.isRequired())
            oss << "}" << endl;

        oss << endl;
    }

    return oss.str();
}

string Class::generateNodeSetter(string memberName, string nodeName) const {
    if(isSimple() && base)
        return base->generateNodeSetter(memberName, nodeName);

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

            if(it->second.cl->isSimple()) {
                //for simple types we're done
                oss << ";" << endl;
            } else {
                //for non-basic types we need to set the contents of the shared_ptr
                oss << "(new " << it->second.cl->getClassname() << ");" << endl;
            }
        } else if(!it->second.cl->isSimple()) {
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
    if(isSimple() && base)
        return base->generateMemberSetter(memberName, nodeName);

    ostringstream oss;

    oss << memberName << "->parseNode(" << nodeName << ");" << endl;

    return oss.str();
}

string Class::getClassname() const {
    if(isSimple() && base)
        return base->getClassname();
    else
        return name.second;
}

string Class::getClassType() const {
    if(isSimple())
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
    os << "#include <xercesc/dom/DOMAttr.hpp>" << endl;
    os << "#include \"XercesString.h\"" << endl;
    os << "#include \"" << className << ".h\"" << endl;

    //no implementation needed for simple types
    if(isSimple())
        return;

    //include headers of all non-basic member types that aren't us
    for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(!it->second.cl->isSimple() && it->second.cl != this)
            os << "#include \"" << it->second.cl->name.second << ".h\"" << endl;

    os << "using namespace std;" << endl;
    os << "using namespace xercesc;" << endl;

    //constructor
    os << className << "::" << className << "() : " << getBaseClassname() << "() {";
    
    //give all basic optional members a default value of zero
    for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(!it->second.isArray() && !it->second.isRequired() && it->second.cl->isSimple())
            os << it->first << " = " << it->second.cl->getDefaultValue() << ";" << endl;

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

    //simple types only need a typedef
    if(isSimple()) {
        os << "typedef " << base->getClassname() << " " << name.second << ";" << endl;
    } else {
        os << "#include \"" << getBaseHeader() << "\"" << endl;

        if(isDocument)
            os << "#include \"XMLDocument.h\"" << endl;

        //non-basic member class prototypes
        for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++)
            if(!it->second.cl->isSimple())
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
            os << "virtual void appendChildren(xercesc::DOMNode *node) const;" << endl;
            os << "virtual void parseNode(xercesc::DOMNode *node);" << endl;
        }

        //members
        for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++) {
            os << it->second.getType() << " " << it->first << ";" << endl;
        }

        os << "};" << endl;
    }
    
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

string Class::getDefaultValue() const {
    if(isSimple() && base)
        return base->getDefaultValue();
    
    return "boost::shared_ptr<" + name.second + ">()";
}

string Class::getTester(string name) const {
    if(isSimple() && base)
        return base->getTester(name);

    return name;
}
