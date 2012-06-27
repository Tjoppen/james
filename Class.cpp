/* Copyright 2011 Tomas Härdin
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
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
        isDocument(false), base(NULL)  {
}

Class::Class(FullName name, ClassType type, FullName baseType) : name(name),
        type(type), isDocument(false), baseType(baseType), base(NULL) {
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

    //make sure members classes add us as their friend
    for(std::list<Member>::iterator it = members.begin(); it != members.end(); it++) {
        //there's no need to befriend ourselves
        if(it->cl && it->cl != this)
            it->cl->friends.insert(getClassname());
    }
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
            oss << base->generateElementSetter("content", nodeWithPostfix, "\t") << endl;
        } else {
            //call base appender
            oss << "\t" << base->getClassname() << "::appendChildren(" << nodeWithPostfix << ");" << endl;
        }
    }
    
    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if (!it->cl)
            continue;

        string name = it->name;
        string setterName = it->name;
        string nodeName = name + "Node";

        if(it != members.begin())
            oss << endl;

        if(it->isArray()) {
            string itName = "it" + variablePostfix;
            setterName = "(*" + itName + ")";
            oss << "\tfor(std::vector<" << it->cl->getClassname() << ">::const_iterator " << itName << " = " << name << ".begin(); " << itName << " != " << name << ".end(); " << itName << "++)" << endl;
        } else if(it->isOptional()) {
            //insert a non-null check
            setterName += ".get()";
            oss << "\tif(" << name << ".isSet())" << endl;
        }

        oss << "\t{" << endl;

        if(it->isAttribute) {
            //attribute
            oss << "\t\tXercesString " << tempWithPostfix << "(\"" << name << "\");" << endl;
            oss << "\t\tDOMAttr *" << nodeName << " = " << nodeWithPostfix << "->getOwnerDocument()->createAttribute(" << tempWithPostfix << ");" << endl;
            oss << it->cl->generateAttributeSetter(setterName, nodeName, "\t\t") << endl;
            oss << "\t\t" << nodeWithPostfix << "->setAttributeNode(" << nodeName << ");" << endl;
        } else {
            //element
            oss << "\t\tXercesString " << tempWithPostfix << "(\"" << name << "\");" << endl;
            oss << "\t\tDOMElement *" << nodeName << " = " << nodeWithPostfix << "->getOwnerDocument()->createElement(" << tempWithPostfix << ");" << endl;
            oss << it->cl->generateElementSetter(setterName, nodeName, "\t\t") << endl;
            oss << "\t\t" << nodeWithPostfix << "->appendChild(" << nodeName << ");" << endl;
        }

        oss << "\t}" << endl;
    }

    return oss.str();
}

string Class::generateElementSetter(string memberName, string nodeName, string tabs) const {
    if(isSimple() && base)
        return base->generateElementSetter(memberName, nodeName, tabs);

    return tabs + memberName + ".appendChildren(" + nodeName + ");";
}

string Class::generateAttributeSetter(string memberName, string attributeName, string tabs) const {
    if(isSimple() && base)
        return base->generateAttributeSetter(memberName, attributeName, tabs);

    throw runtime_error("Tried to generateAttributeSetter() for a non-simple Class");
}

string Class::generateParser() const {
    ostringstream oss;
    string childName = "child" + variablePostfix;
    string nameName = "name" + variablePostfix;

    if(base) {
        if(base->isSimple()) {
            //simpleContent
            oss << base->generateMemberSetter("content", nodeWithPostfix, "\t") << endl;
        } else {
            oss << "\t" << base->getClassname() << "::parseNode(" << nodeWithPostfix << ");" << endl;
        }

        oss << endl;
    }

    oss << "\tfor(DOMNode *" << childName << " = " << nodeWithPostfix << "->getFirstChild(); " << childName << "; " << childName << " = " << childName << "->getNextSibling()) {" << endl;
    oss << "\t\tif(!" << childName << "->getLocalName())" << endl;
    oss << "\t\t\tcontinue;" << endl;
    oss << endl;
    oss << "\t\tXercesString " << nameName << "(" << childName << "->getLocalName());" << endl;
    oss << endl;

    //TODO: replace this with a map<pair<string, DOMNode::ElementType>, void(*)(DOMNode*)> thing?
    //in other words, lookin up parsing function pointers in a map should be faster then all these string comparisons
    bool first = true;

    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if (!it->cl)
            continue;

        if(!it->isAttribute) {
            if(first)
                first = false;
            else
                oss << endl;

            oss << "\t\tif(" << nameName << " == \"" << it->name << "\" && " << childName << "->getNodeType() == DOMNode::ELEMENT_NODE) {" << endl;

            string memberName = it->name;
            if(!it->isRequired()) {
                memberName += tempWithPostfix;
                oss << "\t\t\t" << it->cl->getClassname() << " " << memberName << ";" << endl;
            }

            string childElementName = "childElement" + variablePostfix;
            oss << "\t\t\tDOMElement *" << childElementName << " = dynamic_cast<DOMElement*>(" << childName << ");" << endl;
            oss << it->cl->generateMemberSetter(memberName, childElementName, "\t\t\t");

            if(it->isArray()) {
                oss << "\t\t\t" << it->name << ".push_back(" << memberName << ");" << endl;
            } else if(it->isOptional()) {
                oss << "\t\t\t" << it->name << " = " << memberName << ";" << endl;
            }

            oss << "\t\t}" << endl;
        }
    }

    oss << "\t}" << endl;

    //attributes
    for(std::list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if (!it->cl)
            continue;

        if(it->isAttribute) {
            string attributeNodeName = "attributeNode" + variablePostfix;

            oss << "\t{" << endl;
            oss << "\t\tXercesString " << tempWithPostfix << "(\"" << it->name << "\");" << endl;
            oss << "\t\tif(" << nodeWithPostfix << "->hasAttribute(" << tempWithPostfix << ")) {" << endl;
            oss << "\t\t\tDOMAttr *" << attributeNodeName << " = " << nodeWithPostfix << "->getAttributeNode(" << tempWithPostfix << ");" << endl;

            string attributeName = it->name;

            if(it->isOptional()) {
                attributeName += "Temp";
                oss << "\t\t\t" << it->cl->getClassname() << " " << attributeName << ";" << endl;
            }


            oss << it->cl->generateAttributeParser(attributeName, attributeNodeName, "\t\t\t") << endl;

            if(it->isOptional()) {
                oss << "\t\t\t" << it->name << " = " << attributeName << ";" << endl;
            }

            oss << "\t\t}" << endl << "\t}" << endl;
        }
    }

    return oss.str();
}

string Class::generateMemberSetter(string memberName, string nodeName, string tabs) const {
    if(isSimple() && base)
        return base->generateMemberSetter(memberName, nodeName, tabs);

    ostringstream oss;

    oss << tabs << memberName << ".parseNode(" << nodeName << ");" << endl;

    return oss.str();
}

string Class::generateAttributeParser(string memberName, string attributeName, string tabs) const {
    if(isSimple() && base)
        return base->generateAttributeParser(memberName, attributeName, tabs);

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

bool Class::hasHeader() const {
    return true;
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

    os << endl;
    os << "using namespace std;" << endl;
    os << "using namespace xercesc;" << endl;
    os << "using namespace james;" << endl;
    os << endl;

    if (needsProtectedDefaultConstructor()) {
        if(base && !base->isSimple())
            os << className << "::" << className << "() : " << base->getClassname() << "() {}" << endl;
        else
            os << className << "::" << className << "() {}" << endl;
        os << endl;
    }

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

    os << "\tis >> *this;" << endl;
    os << "}" << endl;
    os << endl;

    //factory method
    os << className << " " << className << "::fromString(const std::string& str) {" << endl;
    os << "\tistringstream iss(str);" << endl;
    os << "\treturn " << className << "(iss);" << endl;
    os << "}" << endl;
    os << endl;

    //string cast operator
    os << className << "::operator std::string () const {" << endl;
    os << "\tostringstream oss;" << endl;
    os << "\toss << *this;" << endl;
    os << "\treturn oss.str();" << endl;
    os << "}" << endl;
    os << endl;

    //getName()
    os << "std::string " << className << "::getName() const {" << endl;
    os << "\treturn \"" << className << "\";" << endl;
    os << "}" << endl;
    os << endl;

    //getNamespace()
    os << "std::string " << className << "::getNamespace() const {" << endl;
    os << "\treturn \"" << name.first << "\";" << endl;
    os << "}" << endl;
    os << endl;

    os << "void " << className << "::appendChildren(xercesc::DOMElement *" << nodeWithPostfix << ") const {" << endl;
    os << generateAppender();
    os << "}" << endl << endl;

    os << "void " << className << "::parseNode(xercesc::DOMElement *" << nodeWithPostfix << ") {" << endl;
    os << generateParser() << endl;
    os << "}" << endl << endl;

    os << "std::ostream& operator<< (std::ostream& os, const " << className << "& obj) {" << endl;
    os << "\treturn james::marshal(os, obj, static_cast<void (james::XMLObject::*)(xercesc::DOMElement*) const>(&" << className << "::appendChildren), obj.getName(), obj.getNamespace());" << endl;
    os << "}" << endl << endl;

    os << "std::istream& operator>> (std::istream& is, " << className << "& obj) {" << endl;
    os << "\treturn james::unmarshal(is, obj, static_cast<void (james::XMLObject::*)(xercesc::DOMElement*)>(&" << className << "::parseNode), obj.getName());" << endl;
    os << "}" << endl << endl;
}

set<string> Class::getIncludedClasses() const {
    set<string> classesToInclude;

    //return classes of any simple non-builtin elements and any required non-simple elements
    for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if (it->cl && ((!it->cl->isBuiltIn() && it->cl->isSimple()) || (it->isRequired() && !it->cl->isSimple())))
            classesToInclude.insert(it->cl->getClassname());

    return classesToInclude;
}

set<string> Class::getPrototypeClasses() const {
    set<string> classesToInclude = getIncludedClasses();
    set<string> classesToPrototype;

    //return the classes of any non-simple non-required elements
    for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(it->cl && classesToInclude.find(it->cl->getClassname()) == classesToInclude.end() && !it->cl->isSimple() && !it->isRequired())
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
        if(base && base->hasHeader())
            os << "#include " << getBaseHeader() << endl;
        
        if(!base || base->isSimple())
            os << "#include <libjames/XMLObject.h>" << endl;

        if(isDocument)
            os << "#include <libjames/XMLDocument.h>" << endl;

        //include member classes that we can't prototype
        set<string> classesToInclude = getIncludedClasses();

        for(set<string>::const_iterator it = classesToInclude.begin(); it != classesToInclude.end(); it++)
            os << "#include \"" << *it << ".h\"" << endl;

        os << endl;

        set<string> classesToPrototype = getPrototypeClasses();

        //member class prototypes, but only for classes that we haven't already included
        for(set<string>::const_iterator it = classesToPrototype.begin(); it != classesToPrototype.end(); it++)
            os << "class " << *it << ";" << endl;

        if(classesToPrototype.size() > 0)
            os << endl;

        if(isDocument)
            os << "class " << className << " : public " << base->getClassname() << ", public james::XMLDocument";
        else if(base && !base->isSimple())
            os << "class " << className << " : public " << base->getClassname();
        else
            os << "class " << className << " : public james::XMLObject";
        
        os << " {" << endl;

        if (needsProtectedDefaultConstructor()) {
            os << "protected:" << endl;
            os << "\t" << className << "();" << endl;
            os << endl;

            if(friends.size()) {
                //add friends
                for(set<string>::const_iterator it = friends.begin(); it != friends.end(); it++)
                    os << "\tfriend class " << *it << ";" << endl;

                os << endl;
            }
        }

        os << "public:" << endl;

        //constructors
        for(list<Constructor>::const_iterator it = constructors.begin(); it != constructors.end(); it++) {
            os << "\t";
            it->writePrototype(os, true);
            os << endl;
        }

        //prototypes
        //add constructor for unmarshalling this document from an istream of string
        os << "\t" << className << "(std::istream& is);" << endl;
        os << endl;

        //factory method for unmarshalling std::string
        //we can't use a constructor since that would conflict with the required
        //element constructor for a type that only has one string element
        os << "\tstatic " << className <<  " fromString(const std::string& str);" << endl;
        os << endl;

        //string cast operator
        os << "\toperator std::string () const;" << endl;

        //getName()
        os << "\tstd::string getName() const;" << endl;

        //getNamespace()
        os << "\tstd::string getNamespace() const;" << endl;
        
        os << "\tvoid appendChildren(xercesc::DOMElement *node) const;" << endl;
        os << "\tvoid parseNode(xercesc::DOMElement *node);" << endl;
        os << endl;

        //simpleContent
        if(base && base->isSimple())
            os << "\t" << base->getClassname() << " content;" << endl;

        //members
        for(list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
            os << "\t";

            //elements of unknown types are shown commented out
            if (!it->cl)
                os << "//";

            if(it->isOptional())
                os << "james::optional<";
            else if(it->isArray())
                os << "std::vector<";

            if (it->cl)
                os << it->cl->getClassname();
            else
                os << it->type.second;

            if(it->isOptional() || it->isArray())
                os << " >";

            os << " " << it->name << ";";

            if (!it->cl)
                os << "\t//" << it->type.first << ":" << it->type.second << " is undefined";

            os << endl;
        }

        os << "};" << endl;
        os << endl;
        os << "std::ostream& operator<< (std::ostream& os, const " << className << "& obj);" << endl;
        os << "std::istream& operator>> (std::istream& is, " << className << "& obj);" << endl;
        os << endl;

        //include classes that we prototyped earlier
        for(set<string>::const_iterator it = classesToPrototype.begin(); it != classesToPrototype.end(); it++)
            os << "#include \"" << *it << ".h\"" << endl;

        if(classesToPrototype.size() > 0)
            os << endl;
    }

    os << "#endif //_" << className << "_H" << endl;
}

bool Class::shouldUseConstReferences() const {
    return true;
}

bool Class::needsProtectedDefaultConstructor() const {
    for (std::list<Constructor>::const_iterator it = constructors.begin(); it != constructors.end(); it++)
        if (it->isDefaultConstructor())
            return false;

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
    //check isBuiltIn() else we end up adding "content" more than once
    if(base && base->isSimple() && base->isBuiltIn()) {
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
        if(ita->cl && (ita->cl->getClassname() != itb->cl->getClassname() || ita->isArray() != itb->isArray()))
            return false;

    return true;
}

bool Class::Constructor::isDefaultConstructor() const {
    return baseArgs.size() + ourArgs.size() == 0;
}

void Class::Constructor::writePrototype(ostream &os, bool withSemicolon) const {
    list<Member> all = getAllArguments();

    os << cl->getClassname() << "(";

    for(list<Member>::const_iterator it = all.begin(); it != all.end(); it++) {
        if (!it->cl)
            continue;

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
        bool first = true;

        for(list<Member>::const_iterator it = baseArgs.begin(); it != baseArgs.end(); it++) {
            if (!it->cl)
                continue;

            if (first)
                first = false;
            else
                os << ", ";

            os << it->name;
        }

        os << ")";
        hasParens = true;
    }

    bool first = true;

    for(list<Member>::const_iterator it = ourArgs.begin(); it != ourArgs.end(); it++) {
        if (!it->cl)
            continue;

        if (first && !hasParens)
            first = false;
        else if(hasParens || !first)
            os << ", ";

        os << it->name << "(" << it->name << ")";
    }

    os << " {" << endl << "}" << endl;
}
