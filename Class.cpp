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
        base(NULL), isDocument(false) {
}

Class::Class(FullName name, ClassType type, FullName baseType) : name(name),
        type(type), base(NULL), isDocument(false), baseType(baseType) {
}

Class::~Class() {
}

bool Class::isSimple() const {
    return type == SIMPLE_TYPE;
}

bool Class::isBuiltIn() const {
    return false;
}

bool Class::hasBase() const {
    return baseType.second.length() > 0;
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
        } else if(it->isOptional()) {
            //insert a non-null check
            setterName += ".get()";
            oss << "if(" << name << ") {" << endl;
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
        
        if(it->isArray() || it->isOptional())
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

    if(base && !base->isSimple())
        oss << base->getClassname() << "::parseNode(node);" << endl;

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
            } else if(it->isOptional()) {
                //optional and simple - parse to temp var before setting
                memberName = "temp";
                oss << it->cl->getClassType() << " " << memberName << ";" << endl;
            }

            oss << "DOMElement *childElement = dynamic_cast<DOMElement*>(child);" << endl;
            oss << it->cl->generateMemberSetter(memberName, "childElement");

            if(it->isArray()) {
                oss << it->name << ".push_back(" << memberName << ");" << endl;
            } else if(it->isOptional() && it->cl->isSimple()) {
                oss << it->name << " = " << memberName << ";" << endl;
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

            string attributeName = it->name;

            if(it->isOptional()) {
                attributeName = "temp";
                oss << it->cl->getClassType() << " " << attributeName << ";" << endl;
            }


            oss << it->cl->generateAttributeParser(attributeName, "attributeNode") << endl;

            if(it->isOptional()) {
                oss << it->name << " = " << attributeName << ";" << endl;
            }

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

string Class::getBaseHeader() const {
    if(base->isSimple())
        return base->getBaseHeader();
    
    return "\"" + base->getClassname() + ".h\"";
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
    if(base)
        os << className << "::" << className << "() : " << base->getClassname() << "() {";
    else
        os << className << "::" << className << "() {";
    
    os << "}" << endl << endl;

    //method implementations
    //unmarshalling constructors
    if(base)
        os << className << "::" << className << "(std::istream& is) : " << base->getClassname() << "() {" << endl;
    else
        os << className << "::" << className << "(std::istream& is) {" << endl;

    os << "is >> *this;" << endl;
    os << "}" << endl;

    if(base)
        os << className << "::" << className << "(const std::string& str) : " << base->getClassname() << "() {" << endl;
    else
        os << className << "::" << className << "(const std::string& str) {" << endl;

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

    //getNamespace()
    os << "std::string " << className << "::getNamespace() const {" << endl;
    os << "return \"" << name.first << "\";" << endl;
    os << "}" << endl;

    os << "void " << className << "::appendChildren(xercesc::DOMElement *node) const {" << endl;

    //call base appender
    if(base && !base->isSimple())
        os << base->getClassname() << "::appendChildren(node);" << endl;

    os << generateAppender() << endl;
    os << "}" << endl << endl;

    os << "void " << className << "::parseNode(xercesc::DOMElement *node) {" << endl;
    os << generateParser() << endl;
    os << "}" << endl << endl;

    //cast operator
    os << className << "::operator boost::shared_ptr<" << className << "> () const {" << endl;
    os << "return boost::shared_ptr<" << className << ">(new " << className << "(*this));" << endl;
    os << "}" << endl;

    //clone()
    os << "boost::shared_ptr<" << className << "> " << className << "::clone() const {" << endl;
    os << "stringstream ss;" << endl;
    os << "ss << *this;" << endl;
    os << "return boost::shared_ptr<" << className<< ">(new " << className << "(ss));" << endl;
    os << "}" << endl;
}

void Class::writeHeader(ostream& os) const {
    ClassName className = name.second;

    os << "#ifndef _" << className << "_H" << endl;
    os << "#define _" << className << "_H" << endl;

    os << "#include <vector>" << endl;

    if(isDocument)
        os << "#include <istream>" << endl;

    os << "#include <boost/shared_ptr.hpp>" << endl;
    os << "#include <boost/optional.hpp>" << endl;
    os << "#include <xercesc/util/XercesDefs.hpp>" << endl;
    os << "XERCES_CPP_NAMESPACE_BEGIN class DOMElement; XERCES_CPP_NAMESPACE_END" << endl;
    
    //simple types only need a typedef
    if(isSimple()) {
        os << "typedef " << base->getClassname() << " " << name.second << ";" << endl;
    } else {
        if(base)
            os << "#include " << getBaseHeader() << endl;
        else
            os << "#include \"XMLObject.h\"" << endl;

        if(isDocument)
            os << "#include \"XMLDocument.h\"" << endl;

        //non-basic member class prototypes
        for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++)
            if(!it->cl->isSimple())
                os << "class " << it->cl->getClassname() << ";" << endl;

        if(isDocument)
            os << "class " << className << " : public " << base->getClassname() << ", public james::XMLDocument";
        else if(base)
            os << "class " << className << " : public " << base->getClassname();
        else
            os << "class " << className << " : public james::XMLObject";
        
        os << " {" << endl;
        os << "public:" << endl;

        os << className << "();" << endl;

        //prototypes
        //add constructor for unmarshalling this document from an istream of string
        os << className << "(std::istream& is);" << endl;
        os << className << "(const std::string& str);" << endl;

        //string cast operator
        os << "operator std::string () const;" << endl;

        //getName()
        os << "std::string getName() const;" << endl;

        //getNamespace()
        os << "std::string getNamespace() const;" << endl;
        
        os << "void appendChildren(xercesc::DOMElement *node) const;" << endl;
        os << "void parseNode(xercesc::DOMElement *node);" << endl;

        //cast operator
        os << "operator boost::shared_ptr<" << className << "> () const;" << endl;

        //clone()
        os << "boost::shared_ptr<" << className << "> clone() const;" << endl;

        //members
        for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
            if(it->isOptional() && it->cl->isSimple())
                os << "boost::optional<";

            os << it->getType();

            if(it->isOptional() && it->cl->isSimple())
                os << " >";

            os << " " << it->name << ";" << endl;
        }

        os << "};" << endl;
    }
    
    os << "#endif //_" << className << "_H" << endl;
}

bool Class::Member::isArray() const {
    return maxOccurs > 1 || maxOccurs == UNBOUNDED;
}

bool Class::Member::isOptional() const {
    return minOccurs == 0 && maxOccurs == 1;
}

string Class::Member::getType() const {
    //vector if array, regular member otherwise
    if(isArray()) {
        return "std::vector<" + cl->getClassType() + " >";
    } else
        return cl->getClassType();
}
