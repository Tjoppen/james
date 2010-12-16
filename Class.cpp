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

extern bool verbose;
extern bool generateDefaultCtor;
extern bool generateRequiredCtor;
extern bool generateRequiredAndVectorsCtor;
extern bool generateAllCtor;

const string variablePostfix = "_james";

const string nodeWithPostfix = "node" + variablePostfix;
const string tempWithPostfix = "temp" + variablePostfix;
const string convertedWithPostfix = "converted" + variablePostfix;
const string ssWithPostfix = "ss" + variablePostfix;

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

void Class::addConstructor(const Constructor& constructor) {
    //first make sure an identical constructor doesn't already exist
    for(list<Constructor>::const_iterator it = constructors.begin(); it != constructors.end(); it++)
        if(it->hasSameSignature(constructor))
            return;

    constructors.push_back(constructor);
}

void Class::doPostResolveInit() {
    //figure out which constructors we need
    if(generateDefaultCtor)             addConstructor(Constructor(this));
    if(generateRequiredCtor)            addConstructor(Constructor(this, false, false));
    if(generateRequiredAndVectorsCtor)  addConstructor(Constructor(this, true,  false));
    if(generateAllCtor)                 addConstructor(Constructor(this, true,  true));

    if(constructors.size() == 0)
        throw runtime_error("No constructors in class " + getClassname());
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

    if(base) {
        if(base->isSimple()) {
            //simpleContent
            oss << base->generateElementSetter("content", nodeWithPostfix) << endl;
        } else {
            //call base appender
            oss << base->getClassname() << "::appendChildren(" << nodeWithPostfix << ");" << endl;
        }
    }
    
    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        string name = it->name;
        string setterName = it->name;
        string nodeName = name + "Node";

        if(it->isArray()) {
            string itName = "it" + variablePostfix;
            setterName = "(*" + itName + ")";
            oss << "for(std::vector<" << it->cl->getClassname() << ">::const_iterator " << itName << " = " << name << ".begin(); " << itName << " != " << name << ".end(); " << itName << "++)" << endl;
        } else if(it->isOptional()) {
            //insert a non-null check
            setterName += ".get()";
            oss << "if(" << name << ")" << endl;
        }

        oss << "{" << endl;

        if(it->isAttribute) {
            //attribute
            oss << "XercesString " << tempWithPostfix << "(\"" << name << "\");" << endl;
            oss << "DOMAttr *" << nodeName << " = " << nodeWithPostfix << "->getOwnerDocument()->createAttribute(" << tempWithPostfix << ");" << endl;
            oss << it->cl->generateAttributeSetter(setterName, nodeName) << endl;
            oss << nodeWithPostfix << "->setAttributeNode(" << nodeName << ");" << endl;
        } else {
            //element
            oss << "XercesString " << tempWithPostfix << "(\"" << name << "\");" << endl;
            oss << "DOMElement *" << nodeName << " = " << nodeWithPostfix << "->getOwnerDocument()->createElement(" << tempWithPostfix << ");" << endl;
            oss << it->cl->generateElementSetter(setterName, nodeName) << endl;
            oss << nodeWithPostfix << "->appendChild(" << nodeName << ");" << endl;
        }

        oss << "}" << endl;
    }

    return oss.str();
}

string Class::generateElementSetter(string memberName, string nodeName) const {
    if(isSimple() && base)
        return base->generateElementSetter(memberName, nodeName);

    return memberName + ".appendChildren(" + nodeName + ");";
}

string Class::generateAttributeSetter(string memberName, string attributeName) const {
    if(isSimple() && base)
        return base->generateAttributeSetter(memberName, attributeName);

    throw runtime_error("Tried to generateAttributeSetter() for a non-simple Class");
}

string Class::generateParser() const {
    ostringstream oss;
    string childName = "child" + variablePostfix;
    string nameName = "name" + variablePostfix;

    if(base) {
        if(base->isSimple()) {
            //simpleContent
            oss << base->generateMemberSetter("content", nodeWithPostfix) << endl;
        } else {
            oss << base->getClassname() << "::parseNode(" << nodeWithPostfix << ");" << endl;
        }
    }

    oss << "for(DOMNode *" << childName << " = " << nodeWithPostfix << "->getFirstChild(); " << childName << "; " << childName << " = " << childName << "->getNextSibling()) {" << endl;
    oss << "if(!" << childName << "->getLocalName()) continue;" << endl;
    oss << "XercesString " << nameName << "(" << childName << "->getLocalName());" << endl;

    //TODO: replace this with a map<pair<string, DOMNode::ElementType>, void(*)(DOMNode*)> thing?
    //in other words, lookin up parsing function pointers in a map should be faster then all these string comparisons
    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if(!it->isAttribute) {
            oss << "if(" << nameName << " == \"" << it->name << "\" && " << childName << "->getNodeType() == DOMNode::ELEMENT_NODE) {" << endl;

            string memberName = it->name;
            if(!it->isRequired()) {
                memberName += tempWithPostfix;
                oss << it->cl->getClassname() << " " << memberName << ";" << endl;
            }

            string childElementName = "childElement" + variablePostfix;
            oss << "DOMElement *" << childElementName << " = dynamic_cast<DOMElement*>(" << childName << ");" << endl;
            oss << it->cl->generateMemberSetter(memberName, childElementName);

            if(it->isArray()) {
                oss << it->name << ".push_back(" << memberName << ");" << endl;
            } else if(it->isOptional()) {
                oss << it->name << " = " << memberName << ";" << endl;
            }

            oss << "}" << endl;
        }
    }

    oss << "}" << endl;

    //attributes
    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if(it->isAttribute) {
            string attributeNodeName = "attributeNode" + variablePostfix;

            oss << "{" << endl;
            oss << "XercesString " << tempWithPostfix << "(\"" << it->name << "\");" << endl;
            oss << "if(" << nodeWithPostfix << "->hasAttribute(" << tempWithPostfix << ")) {" << endl;
            oss << "DOMAttr *" << attributeNodeName << " = " << nodeWithPostfix << "->getAttributeNode(" << tempWithPostfix << ");" << endl;

            string attributeName = it->name;

            if(it->isOptional()) {
                attributeName += "Temp";
                oss << it->cl->getClassname() << " " << attributeName << ";" << endl;
            }


            oss << it->cl->generateAttributeParser(attributeName, attributeNodeName) << endl;

            if(it->isOptional()) {
                oss << it->name << " = " << attributeName << ";" << endl;
            }

            oss << "}" << endl << "}" << endl;
        }
    }

    return oss.str();
}

string Class::generateMemberSetter(string memberName, string nodeName) const {
    if(isSimple() && base)
        return base->generateMemberSetter(memberName, nodeName);

    ostringstream oss;

    oss << memberName << ".parseNode(" << nodeName << ");" << endl;

    return oss.str();
}

string Class::generateAttributeParser(string memberName, string attributeName) const {
    if(isSimple() && base)
        return base->generateAttributeParser(memberName, attributeName);

    throw runtime_error("Tried to generateAttributeParser() for a non-simple Class");
}

string Class::getClassname() const {
    return name.second;
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
    os << "#include <libjames/XercesString.h>" << endl;
    os << "#include \"" << className << ".h\"" << endl;

    //no implementation needed for simple types
    if(isSimple())
        return;

    os << "using namespace std;" << endl;
    os << "using namespace xercesc;" << endl;
    os << "using namespace james;" << endl;

    //constructors
    for(list<Constructor>::const_iterator it = constructors.begin(); it != constructors.end(); it++) {
        it->writeBody(os);
        os << endl;
    }

    //method implementations
    //unmarshalling constructors
    if(base && !base->isSimple())
        os << className << "::" << className << "(std::istream& is) : " << base->getClassname() << "() {" << endl;
    else
        os << className << "::" << className << "(std::istream& is) {" << endl;

    os << "is >> *this;" << endl;
    os << "}" << endl;

    //factory method
    os << className << " " << className << "::fromString(const std::string& str) {" << endl;
    os << "istringstream iss(str);" << endl;
    os << "return " << className << "(iss);" << endl;
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

    os << "void " << className << "::appendChildren(xercesc::DOMElement *" << nodeWithPostfix << ") const {" << endl;
    os << generateAppender() << endl;
    os << "}" << endl << endl;

    os << "void " << className << "::parseNode(xercesc::DOMElement *" << nodeWithPostfix << ") {" << endl;
    os << generateParser() << endl;
    os << "}" << endl << endl;

    //clone()
    os << className << " " << className << "::clone() const {" << endl;
    os << "stringstream " << ssWithPostfix <<";" << endl;
    os << ssWithPostfix << " << *this;" << endl;
    os << "return " << className << "(" << ssWithPostfix << ");" << endl;
    os << "}" << endl;
}

set<string> Class::getIncludedClasses() const {
    set<string> classesToInclude;

    //return classes of any simple non-builtin elements and any required non-simple elements
    for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(!it->cl->isBuiltIn() && it->cl->isSimple() || it->isRequired() && !it->cl->isSimple())
            classesToInclude.insert(it->cl->getClassname());

    return classesToInclude;
}

set<string> Class::getPrototypeClasses() const {
    set<string> classesToInclude = getIncludedClasses();
    set<string> classesToPrototype;

    //return the classes of any non-simple non-required elements
    for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(classesToInclude.find(it->cl->getClassname()) == classesToInclude.end() && !it->cl->isSimple() && !it->isRequired())
            classesToPrototype.insert(it->cl->getClassname());

    return classesToPrototype;
}

void Class::writeHeader(ostream& os) const {
    ClassName className = name.second;

    os << "#ifndef _" << className << "_H" << endl;
    os << "#define _" << className << "_H" << endl;

    os << "#include <vector>" << endl;

    if(isDocument)
        os << "#include <istream>" << endl;

    os << "#include <xercesc/util/XercesDefs.hpp>" << endl;
    os << "XERCES_CPP_NAMESPACE_BEGIN class DOMElement; XERCES_CPP_NAMESPACE_END" << endl;
    os << "#include <libjames/HexBinary.h>" << endl;
    os << "#include <libjames/optional.h>" << endl;
    
    //simple types only need a typedef
    if(isSimple()) {
        os << "typedef " << base->getClassname() << " " << name.second << ";" << endl;
    } else {
        if(base)
            os << "#include " << getBaseHeader() << endl;
        
        if(!base || base->isSimple())
            os << "#include <libjames/XMLObject.h>" << endl;

        if(isDocument)
            os << "#include <libjames/XMLDocument.h>" << endl;

        //include member classes that we can't prototype
        set<string> classesToInclude = getIncludedClasses();

        for(set<string>::const_iterator it = classesToInclude.begin(); it != classesToInclude.end(); it++)
            os << "#include \"" << *it << ".h\"" << endl;

        set<string> classesToPrototype = getPrototypeClasses();

        //member class prototypes, but only for classes that we haven't already included
        for(set<string>::const_iterator it = classesToPrototype.begin(); it != classesToPrototype.end(); it++)
            os << "class " << *it << ";" << endl;

        if(isDocument)
            os << "class " << className << " : public " << base->getClassname() << ", public james::XMLDocument";
        else if(base && !base->isSimple())
            os << "class " << className << " : public " << base->getClassname();
        else
            os << "class " << className << " : public james::XMLObject";
        
        os << " {" << endl;
        os << "public:" << endl;

        //constructors
        for(list<Constructor>::const_iterator it = constructors.begin(); it != constructors.end(); it++) {
            it->writePrototype(os, true);
            os << endl;
        }

        //prototypes
        //add constructor for unmarshalling this document from an istream of string
        os << className << "(std::istream& is);" << endl;

        //factory method for unmarshalling std::string
        //we can't use a constructor since that would conflict with the required
        //element constructor for a type that only has one string element
        os << "static " << className <<  " fromString(const std::string& str);" << endl;

        //string cast operator
        os << "operator std::string () const;" << endl;

        //getName()
        os << "std::string getName() const;" << endl;

        //getNamespace()
        os << "std::string getNamespace() const;" << endl;
        
        os << "void appendChildren(xercesc::DOMElement *node) const;" << endl;
        os << "void parseNode(xercesc::DOMElement *node);" << endl;

        //clone()
        os << className << " clone() const;" << endl;

        //simpleContent
        if(base && base->isSimple())
            os << base->getClassname() << " content;" << endl;

        //members
        for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
            if(it->isOptional())
                os << "james::optional<";
            else if(it->isArray())
                os << "std::vector<";

            os << it->cl->getClassname();

            if(it->isOptional() || it->isArray())
                os << " >";

            os << " " << it->name << ";" << endl;
        }

        os << "};" << endl;

        //include classes that we prototyped earlier
        for(set<string>::const_iterator it = classesToPrototype.begin(); it != classesToPrototype.end(); it++)
            os << "#include \"" << *it << ".h\"" << endl;
    }
    
    os << "#endif //_" << className << "_H" << endl;
}

bool Class::shouldUseConstReferences() const {
    return true;
}

bool Class::Member::isArray() const {
    return maxOccurs > 1 || maxOccurs == UNBOUNDED;
}

bool Class::Member::isOptional() const {
    return minOccurs == 0 && maxOccurs == 1;
}

bool Class::Member::isRequired() const {
    return !isArray() && !isOptional();
}

std::list<Class::Member> Class::getElements(bool includeBase, bool vectors, bool optionals) const {
    std::list<Member> ret;

    if(includeBase && base)
        ret = base->getElements(true, vectors, optionals);

    //regard the contents of a complexType with simpleContents as a required
    //element named "content" since we already have that as an element
    if(base && base->isSimple()) {
        Member contentMember;
        contentMember.name = "content";
        contentMember.cl = base;
        contentMember.minOccurs = contentMember.maxOccurs = 1;

        ret.push_back(contentMember);
    }

    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(it->isRequired() || (it->isArray() && vectors) || (it->isOptional() && optionals))
            ret.push_back(*it);

    return ret;
}

Class::Constructor::Constructor(Class *cl) : cl(cl) {
}

Class::Constructor::Constructor(Class *cl, bool vectors, bool optionals) :
        cl(cl) {
    if(cl->base)
        baseArgs = cl->base->getElements(true, vectors, optionals);

    ourArgs = cl->getElements(false, vectors, optionals);
}

list<Class::Member> Class::Constructor::getAllArguments() const {
    list<Class::Member> ret = baseArgs;

    ret.insert(ret.end(), ourArgs.begin(), ourArgs.end());

    return ret;
}

bool Class::Constructor::hasSameSignature(const Constructor& other) const {
    list<Member> a = getAllArguments();
    list<Member> b = other.getAllArguments();

    if(a.size() != b.size())
        return false;

    list<Member>::iterator ita = a.begin(), itb = b.begin();

    //return false if the arguments in any position are of different types or
    //if one is an array but the other isn't
    for(; ita != a.end(); ita++, itb++)
        if(ita->cl->getClassname() != itb->cl->getClassname() || ita->isArray() != itb->isArray())
            return false;

    return true;
}

void Class::Constructor::writePrototype(ostream &os, bool withSemicolon) const {
    list<Member> all = getAllArguments();

    os << cl->getClassname() << "(";

    for(list<Member>::const_iterator it = all.begin(); it != all.end(); it++) {
        if(it != all.begin())
            os << ", ";

        if(it->isArray())
            os << "const std::vector<";
        else if(it->cl->shouldUseConstReferences())
            os << "const ";

        os << it->cl->getClassname();

        if(it->isArray())
            os << " >&";
        else if(it->cl->shouldUseConstReferences())
            os << "&";

        os << " " << it->name;
    }

    os << ")";

    if(withSemicolon)
        os << ";";
}

void Class::Constructor::writeBody(ostream &os) const {
    list<Member> all = getAllArguments();

    os << cl->getClassname() << "::";

    writePrototype(os, false);

    if(all.size() > 0 || (cl->base && !cl->base->isSimple()))
        os << " :" << endl << "\t";

    bool hasParens = false;

    if(cl->base && !cl->base->isSimple()) {
        //pass the base class' elements
        os << cl->base->getClassname() << "(";

        for(list<Member>::const_iterator it = baseArgs.begin(); it != baseArgs.end(); it++) {
            if(it != baseArgs.begin())
                os << ", ";

            os << it->name;
        }

        os << ")";
        hasParens = true;
    }

    for(list<Member>::const_iterator it = ourArgs.begin(); it != ourArgs.end(); it++) {
        if(hasParens || it != ourArgs.begin())
            os << ", ";

        os << it->name << "(" << it->name << ")";
    }

    os << " {" << endl << "}" << endl;
}
