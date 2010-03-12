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

extern bool verbose;

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

std::list<Class::Member>::iterator Class::findMember(std::string name) {
    for(std::list<Member>::iterator it = members.begin(); it != members.end(); it++)
        if(it->name == name)
            return it;

    return members.end();
}

void Class::addMember(Member memberInfo) {
    if(findMember(memberInfo.name) != members.end())
        throw runtime_error("Member " + memberInfo.name + " defined more than once in " + this->name.second);

    if(verbose) cerr << this->name.second << " got " << memberInfo.type.first << ":" << memberInfo.type.second << " " << memberInfo.name << ". Occurance: ";

    if(memberInfo.maxOccurs == UNBOUNDED) {
        if(verbose) cerr << "at least " << memberInfo.minOccurs;
    } else if(memberInfo.minOccurs == memberInfo.maxOccurs) {
        if(verbose) cerr << "exactly " << memberInfo.minOccurs;
    } else {
        if(verbose) cerr << "between " << memberInfo.minOccurs << "-" << memberInfo.maxOccurs;
    }

    if(verbose) cerr << endl;

    members.push_back(memberInfo);
}

/**
 * Default implementation of generateAppender()
 */
string Class::generateAppender() const {
    ostringstream oss;
    
    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        string name = it->name;
        string setterName = it->name;
        string nodeName = name + "Node";

        if(it->isArray()) {
            setterName = "(*it)";
            oss << "for(" << it->getType() << "::const_iterator it = " << name << ".begin(); it != " << name << ".end(); it++) {" << endl;
        } else if(!it->isRequired()) {
            //insert a non-null check
            oss << "if(" << it->cl->getTester(name) << ") {" << endl;
        } else {
            //required member - check for its existance
        }

        if(it->isAttribute) {
            //attribute
            oss << "DOMAttr *" << nodeName << " = node->getOwnerDocument()->createAttribute(XercesString(\"" << name << "\"));" << endl;
            oss << it->cl->generateAttributeSetter(setterName, nodeName) << endl;
            oss << "node->setAttributeNode(" << nodeName << ");" << endl;
        } else {
            //element
            oss << "DOMElement *" << nodeName << " = node->getOwnerDocument()->createElement(XercesString(\"" << name << "\"));" << endl;
            oss << it->cl->generateElementSetter(setterName, nodeName) << endl;
            oss << "node->appendChild(" << nodeName << ");" << endl;
        }

        if(it->isArray() || !it->isRequired())
            oss << "}" << endl;

        oss << endl;
    }

    return oss.str();
}

string Class::generateElementSetter(string memberName, string nodeName) const {
    if(isSimple() && base)
        return base->generateElementSetter(memberName, nodeName);

    return memberName + "->appendChildren(" + nodeName + ");";
}

string Class::generateAttributeSetter(string memberName, string attributeName) const {
    if(isSimple() && base)
        return base->generateAttributeSetter(memberName, attributeName);

    throw runtime_error("Tried to generateAttributeSetter() for a non-simple Class");
}

string Class::generateParser() const {
    ostringstream oss;

    if(hasBase && !base->isSimple())
        oss << getBaseClassname() << "::parseNode(node);" << endl;

    oss << "for(DOMNode *child = node->getFirstChild(); child; child = child->getNextSibling()) {" << endl;
    oss << "if(!child->getLocalName()) continue;" << endl;
    oss << "XercesString name(child->getLocalName());" << endl;

    //TODO: replace this with a map<pair<string, DOMNode::ElementType>, void(*)(DOMNode*)> thing?
    //in other words, lookin up parsing function pointers in a map should be faster then all these string comparisons
    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if(!it->isAttribute) {
            oss << "if(name == \"" << it->name << "\" && child->getNodeType() == DOMNode::ELEMENT_NODE) {" << endl;

            string memberName = it->name;
            if(it->isArray()) {
                memberName = "temp";
                oss << it->cl->getClassType() << " " << memberName;

                if(it->cl->isSimple()) {
                    //for simple types we're done
                    oss << ";" << endl;
                } else {
                    //for non-basic types we need to set the contents of the shared_ptr
                    oss << "(new " << it->cl->getClassname() << ");" << endl;
                }
            } else if(!it->cl->isSimple()) {
                //add check and allocation of shared_ptr
                oss << "if(!" << memberName << ") " << memberName << " = boost::shared_ptr<" << it->cl->getClassname() << ">(new " << it->cl->getClassname() << ");" << endl;
            }

            oss << "DOMElement *childElement = dynamic_cast<DOMElement*>(child);" << endl;
            oss << it->cl->generateMemberSetter(memberName, "childElement");

            if(it->isArray()) {
                oss << it->name << ".push_back(" << memberName << ");" << endl;
            }

            oss << "}" << endl;
        }
    }

    oss << "}" << endl;

    //attributes
    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if(it->isAttribute) {
            oss << "if(node->hasAttribute(XercesString(\"" << it->name << "\"))) {" << endl;
            oss << "DOMAttr *attributeNode = node->getAttributeNode(XercesString(\"" << it->name << "\"));" << endl;
            oss << it->cl->generateAttributeParser(it->name, "attributeNode") << endl;
            oss << "}" << endl;
        }
    }

    return oss.str();
}

string Class::generateMemberSetter(string memberName, string nodeName) const {
    if(isSimple() && base)
        return base->generateMemberSetter(memberName, nodeName);

    ostringstream oss;

    oss << memberName << "->parseNode(" << nodeName << ");" << endl;

    return oss.str();
}

string Class::generateAttributeParser(string memberName, string attributeName) const {
    if(isSimple() && base)
        return base->generateAttributeParser(memberName, attributeName);

    throw runtime_error("Tried to generateAttributeParser() for a non-simple Class");
}

string Class::generateInsertClones() const {
    //TODO: implement generateCloner()
    ostringstream oss;
    ClassName className = name.second;

    for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        string name = it->name;
        string sourceName = "this->" + it->name;
        string targetName = "ret->" + it->name;
        
        if(it->isArray()) {
            sourceName = "(*it)";
            targetName = "temp";
            oss << "for(" << it->getType() << "::const_iterator it = " << name << ".begin(); it != " << name << ".end(); it++) {" << endl;
            oss << it->cl->getClassType() << " " << it->cl->generateMemberCloner(targetName, sourceName) << endl;
            oss << "ret->" << it->name << ".push_back(temp);" << endl;
            oss << "}" << endl;
        } else {
            oss << it->cl->generateMemberCloner(targetName, sourceName) << endl;
        }
    }

    oss << "return ret;" << endl;

    return oss.str();
}

string Class::generateMemberCloner(string cloneName, string memberName) const {
    if(base)
        return base->generateMemberCloner(cloneName, memberName);
    else if(isBuiltIn())
        return cloneName + " = " + memberName + ";";
    else
        return cloneName + " = " + memberName + "->clone();";
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
    if(base && base->isSimple())
        return base->getBaseHeader();
    
    return hasBase ? "\"" + base->getClassname() + ".h\"" : "\"XMLObject.h\"";
}

void Class::writeImplementation(ostream& os) const {
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
    for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(!it->cl->isSimple() && it->cl != this)
            os << "#include \"" << it->cl->name.second << ".h\"" << endl;

    os << "using namespace std;" << endl;
    os << "using namespace xercesc;" << endl;
    os << "using namespace james;" << endl;

    //constructors
    os << className << "::" << className << "() : " << getBaseClassname() << "() {";
    
    //give all basic optional members a default value of zero
    for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(!it->isArray() && !it->isRequired() && it->cl->isSimple())
            os << it->name << " = " << it->cl->getDefaultValue() << ";" << endl;

    os << "}" << endl << endl;

    //method implementations
    if(isDocument) {
        //unmarshalling constructors
        os << className << "::" << className << "(std::istream& is) : " << getBaseClassname() << "() {" << endl;
        os << "is >> *this;" << endl;
        os << "}" << endl;

        os << className << "::" << className << "(const std::string& str) : " << getBaseClassname() << "() {" << endl;
        os << "istringstream iss(str);" << endl;
        os << "iss >> *this;" << endl;
        os << "}" << endl;

        //string cast operator
        os << className << "::operator std::string () const {" << endl;
        os << "ostringstream oss;" << endl;
        os << "oss << *this;" << endl;
        os << "return oss.str();" << endl;
        os << "}" << endl;

        //getName()
        os << "std::string " << className << "::getName() const {" << endl;
        os << "return \"" << className << "\";" << endl;
        os << "}" << endl;
    } else {
        os << "void " << className << "::appendChildren(xercesc::DOMElement *node) const {" << endl;
        
        //call base appender
        if(hasBase && !base->isSimple())
            os << getBaseClassname() << "::appendChildren(node);" << endl;

        os << generateAppender() << endl;
        os << "}" << endl << endl;

        os << "void " << className << "::parseNode(xercesc::DOMElement *node) {" << endl;
        os << generateParser() << endl;
        os << "}" << endl << endl;

        //clone()
        os << "boost::shared_ptr<" << className << "> " << className << "::clone() const {" << endl;
        os << "boost::shared_ptr<" << className << "> ret(new " << className << ");" << endl;

        if(base)
            os << base->name.second << "::insertClones(ret);" << endl;

        os << "insertClones(ret);" << endl;
        os << "return ret;" << endl;
        os << "}" << endl;

        //insertClones()
        os << "boost::shared_ptr<" << className << "> " << className << "::insertClones(boost::shared_ptr<" << className << "> ret) const {" << endl;
        os << generateInsertClones() << endl;
        os << "}" << endl;

        //clone-cast operator
        os << className << "::operator boost::shared_ptr<" << className << "> () const {" << endl;
        os << "return clone();" << endl;
        os << "}" << endl;
    }
}

void Class::writeHeader(ostream& os) const {
    ClassName className = name.second;

    os << "#ifndef _" << className << "_H" << endl;
    os << "#define _" << className << "_H" << endl;

    os << "#include <vector>" << endl;

    if(isDocument)
        os << "#include <istream>" << endl;

    os << "#include <boost/shared_ptr.hpp>" << endl;

    //simple types only need a typedef
    if(isSimple()) {
        os << "typedef " << base->getClassname() << " " << name.second << ";" << endl;
    } else {
        os << "#include " << getBaseHeader() << endl;

        if(isDocument)
            os << "#include \"XMLDocument.h\"" << endl;

        //non-basic member class prototypes
        for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++)
            if(!it->cl->isSimple())
                os << "class " << it->cl->getClassname() << ";" << endl;

        os << "class " << className << " : public " << getBaseClassname();

        if(isDocument)
            os << ", public james::XMLDocument" << endl;

        os << " {" << endl;
        os << "public:" << endl;

        os << className << "();" << endl;

        //prototypes
        if(isDocument) {
            //add constructor for unmarshalling this document from an istream of string
            os << className << "(std::istream& is);" << endl;
            os << className << "(const std::string& str);" << endl;

            //string cast operator
            os << "operator std::string () const;" << endl;

            //getName()
            os << "std::string getName() const;" << endl;
        } else {
            os << "virtual void appendChildren(xercesc::DOMElement *node) const;" << endl;
            os << "virtual void parseNode(xercesc::DOMElement *node);" << endl;

            //clone()
            os << "boost::shared_ptr<" << className << "> clone() const;" << endl;

            //clone-cast operator
            os << "operator boost::shared_ptr<" << className << "> () const;" << endl;

            //insertClones()
            os << "protected:" << endl;
            os << "boost::shared_ptr<" << className << "> insertClones(boost::shared_ptr<" << className << "> ret) const;" << endl;

            os << "public:" << endl;
        }

        //members
        for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
            os << it->getType() << " " << it->name << ";" << endl;
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
