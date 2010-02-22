/* 
 * File:   main.cpp
 * Author: tjoppen
 *
 * Created on February 12, 2010, 3:53 PM
 */

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>

#include "main.h"
#include "XercesString.h"
#include "Class.h"
#include "BuiltInClasses.h"

using namespace std;
using namespace boost;
using namespace xercesc;

static void printUsage() {
    cout << "USAGE: james output-dir list-of-XSL-documents" << endl;
    cout << " Generates C++ classes for marshalling and unmarshalling XML to C++ objects according to the given schemas." << endl;
    cout << " Files are output in the specified output directory and are named type.h and type.cpp" << endl;
}

//maps namespace abbreviation to their full URIs
map<string, string> nsLUT;

//collection of all generated classes
map<FullName, shared_ptr<Class> > classes;

static shared_ptr<Class> addClass(shared_ptr<Class> cl) {
    if(classes.find(cl->name) != classes.end())
        throw runtime_error(cl->name.first + ":" + cl->name.second + " defined more than once");

    return classes[cl->name] = cl;
}

static string fixIdentifier(string str) {
    //strip any bad characters such as dots, colons, semicolons..
    string ret;

    for(int x = 0; x < str.size(); x++) {
        char c = str[x];

        if((c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'Z') ||
                c == '_')
            ret += c;
        else
            ret += "_";
    }

    if(ret == "default")
        ret += "_";

    return ret;
}

static string lookupNamespace(string typeName, string defaultNamespace) {
    //figures out namespace URI of given type
    size_t pos = typeName.find_last_of(':');

    if(pos == string::npos)
        return defaultNamespace;

    return nsLUT[typeName.substr(0, pos)];
}

static string stripNamespace(string typeName) {
    //strip namespace part of string
    //makes "xs:int" into "int", "tns:Foo" into "Foo" etc.
    size_t pos = typeName.find_last_of(':');

    if(pos == string::npos)
        return typeName;
    else
        return typeName.substr(pos + 1, typeName.length() - pos - 1);
}

static FullName toFullName(string typeName, string defaultNamespace = "") {
    //looks up and strips namespace from typeName and builds a FullName of the result
    return FullName(lookupNamespace(typeName, defaultNamespace), stripNamespace(typeName));
}

static DOMElement *getExpectedChildElement(DOMNode *parent, string childName) {
    for(DOMNode *child = parent->getFirstChild(); child; child = child->getNextSibling()) {
        if(child->getNodeType() == DOMNode::ELEMENT_NODE && child->getLocalName() && X(child->getLocalName()) == childName) {
            DOMElement *childElement = dynamic_cast<DOMElement*>(child);
            CHECK(childElement);

            return childElement;
        }
    }

    throw runtime_error((string)X(parent->getLocalName()) + " missing expected child element " + childName);
}

static vector<DOMElement*> getChildElements(DOMElement *parent) {
    vector<DOMElement*> ret;
    
    for(DOMNode *child = parent->getFirstChild(); child; child = child->getNextSibling()) {
        if(child->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement *childElement = dynamic_cast<DOMElement*>(child);
            CHECK(childElement);

            ret.push_back(childElement);
        }
    }

    return ret;
}

static vector<DOMElement*> getChildElementsByTagName(DOMElement *parent, string childName) {
    vector<DOMElement*> childElements = getChildElements(parent);
    vector<DOMElement*> ret;

    for(int x = 0; x < childElements.size(); x++) {
        if(childElements[x]->getLocalName() && X(childElements[x]->getLocalName()) == childName) {
            ret.push_back(childElements[x]);
        }
    }

    return ret;
}

static void parseComplexType(DOMElement *element, FullName fullName, shared_ptr<Class> cl = shared_ptr<Class>());

static void parseSequence(DOMElement *parent, DOMElement *sequence, shared_ptr<Class> cl, bool choice = false) {
    //we expect to see a whole bunch of <element>s here
    //if choice is true then this is a choice sequence - every element is optional
    CHECK(parent);
    CHECK(sequence);

    vector<DOMElement*> children = getChildElementsByTagName(sequence, "element");
    
    for(int x = 0; x < children.size(); x++) {
        DOMElement *child = children[x];
            
        int minOccurs = 1;
        int maxOccurs = 1;

        XercesString typeStr("type");
        XercesString minOccursStr("minOccurs");
        XercesString maxOccursStr("maxOccurs");
        string name = fixIdentifier(X(child->getAttribute(X("name"))));

        if(child->hasAttribute(minOccursStr)) {
            stringstream ss;
            ss << X(child->getAttribute(minOccursStr));
            ss >> minOccurs;
        }

        if(child->hasAttribute(maxOccursStr)) {
            XercesString str(child->getAttribute(maxOccursStr));

            if(str == "unbounded")
                maxOccurs = UNBOUNDED;
            else {
                stringstream ss;
                ss << str;
                ss >> maxOccurs;
            }
        }

        //all choice elements are optional
        if(choice) {
            if(maxOccurs > 1)
                throw runtime_error("maxOccurs > 1 specified for choice element");

            minOccurs = 0;
            maxOccurs = 1;
        }

        if(child->hasAttribute(typeStr)) {
            //has type == end point - add as member of cl
            Class::Member info;

            //assume in same namespace for now
            info.type = toFullName(X(child->getAttribute(typeStr)));
            info.minOccurs = minOccurs;
            info.maxOccurs = maxOccurs;
            info.isAttribute = false;

            cl->addMember(name, info);
        } else {
            //no type - anonymous subtype
            //generate name
            FullName subName(cl->name.first, cl->name.second + "_" + (string)name);

            //expect <complexType> sub-tag
            parseComplexType(getExpectedChildElement(child, "complexType"), subName);

            Class::Member info;
            info.type = subName;
            info.minOccurs = minOccurs;
            info.maxOccurs = maxOccurs;
            info.isAttribute = false;

            cl->addMember(name, info);
        }
    }
}

static void parseComplexType(DOMElement *element, FullName fullName, shared_ptr<Class> cl) {
    //we handle two cases with <complexType>:
    //child is <sequence>
    //child is <complexContent> - expect grandchild <extension>
    CHECK(element);

    //bootstrap Class pointer in case we didn't come from the recursive <extension> call below
    if(!cl)
        cl = addClass(shared_ptr<Class>(new Class(fullName, Class::COMPLEX_TYPE)));
    
    vector<DOMElement*> childElements = getChildElements(element);

    for(int x = 0; x < childElements.size(); x++) {
        DOMElement *child = childElements[x];
        XercesString name(child->getLocalName());

        if(name == "sequence") {
            parseSequence(element, child, cl);
        } else if(name == "choice" || name == "all") {
            if(child->hasAttribute(X("minOccurs")) || child->hasAttribute(X("maxOccurs")))
                throw runtime_error("minOccurs/maxOccurs not currently supported in <choice>/<all> types");

            parseSequence(element, child, cl, true);
        } else if(name == "complexContent" || name == "simpleContent") {
            DOMElement *extension = getExpectedChildElement(child, "extension");
            
            if(!extension->hasAttribute(X("base")))
                throw runtime_error("Extension missing expected attribute base");
            
            //set base type and treat the extension as complexType itself
            FullName base = toFullName(X(extension->getAttribute(X("base"))));

            cl->baseType = base;
            cl->hasBase = true;

            parseComplexType(extension, fullName, cl);
        } else if(name == "attribute") {
            bool optional = false;

            if(!child->hasAttribute(X("type")))
                throw runtime_error("<attribute> missing expected attribute 'type'");

            if(!child->hasAttribute(X("name")))
                throw runtime_error("<attribute> missing expected attribute 'name'");

            string attributeName = fixIdentifier(X(child->getAttribute(X("name"))));

            FullName type = toFullName(X(child->getAttribute(X("type"))));

            //check for optional use
            if(child->hasAttribute(X("use")) && X(child->getAttribute(X("use"))) == "optional")
                optional = true;

            Class::Member info;
            info.type = type;
            info.isAttribute = true;
            info.minOccurs = optional ? 0 : 1;
            info.maxOccurs = 1;

            cl->addMember(attributeName, info);
        } else {
            throw runtime_error("Unknown complexType child of type " + (string)name);
        }
    }
}

static void parseSimpleType(DOMElement *element, FullName fullName) {
    //expect a <restriction> child element
    CHECK(element);

    DOMElement *restriction = getExpectedChildElement(element, "restriction");

    if(!restriction->hasAttribute(X("base")))
        throw runtime_error("simpleType restriction lacks expected attribute 'base'");

    //convert xs:string and the like to their respective FullName
    FullName baseName = toFullName(X(restriction->getAttribute(X("base"))));

    //add class and return
    addClass(shared_ptr<Class>(new Class(fullName, Class::SIMPLE_TYPE, baseName)));
}

static void parseElement(DOMElement *element, string tns) {
    CHECK(element);

    XercesString nodeNs(element->getNamespaceURI());
    XercesString nodeName(element->getLocalName());

    if(nodeNs != XSL || (
            nodeName != "complexType" &&
            nodeName != "element" &&
            nodeName != "simpleType"))
        return;

    //<complexType>, <element> or <simpleType>
    //figure out its class name
    XercesString name(element->getAttribute(X("name")));
    FullName fullName(tns, name);

    cout << "\t" << "new " << nodeName << ": " << fullName.second << endl;

    if(nodeName == "complexType")
        parseComplexType(element, fullName);
    else if(nodeName == "element") {
        //if <element> is missing type, then its type is anonymous
        FullName type;

        if(!element->hasAttribute(X("type"))) {
            //anonymous element type. derive it using expected <complexType>
            type = FullName(tns, fullName.second + "Type");

            parseComplexType(getExpectedChildElement(element, "complexType"), type);
        } else
            type = toFullName(X(element->getAttribute(X("type"))), tns);

        addClass(shared_ptr<Class>(new Class(fullName, Class::COMPLEX_TYPE, type)))->isDocument = true;
    } else if(nodeName == "simpleType") {
        parseSimpleType(element, fullName);
    }
}

static void work(string outputDir, const vector<string>& schemaNames) {
    XercesDOMParser parser;
    parser.setDoNamespaces(true);

    for(size_t x = 0; x < schemaNames.size(); x++) {
        string name = schemaNames[x];
        parser.parse(name.c_str());

        DOMDocument *document = parser.getDocument();
        DOMElement *root = document->getDocumentElement();

        DOMAttr *targetNamespace = root->getAttributeNode(X("targetNamespace"));
        CHECK(targetNamespace);
        string tns = X(targetNamespace->getValue());

        //HACKHACK: we should handle NS lookup properly
        nsLUT["tns"] = tns;
        
        cout << "Target namespace: " << tns << endl;

        vector<DOMElement*> elements = getChildElements(root);

        for(int x = 0; x < elements.size(); x++)
            parseElement(elements[x], tns);
    }

    cout << "About to make second pass. Pointing class members to referenced classes, or failing if any undefined classes are encountered." << endl;

    //make second pass through classes and set all member and base class pointers correctly
    //this has the side effect of catching any undefined classes
    for(map<FullName, shared_ptr<Class> >::iterator it = classes.begin(); it != classes.end(); it++) {
        if(it->second->hasBase) {
            if(classes.find(it->second->baseType) == classes.end())
                throw runtime_error("Undefined base type " + it->second->baseType.first + ":" + it->second->baseType.second + " of " + it->second->name.first + ":" + it->second->name.second);

            it->second->base = classes[it->second->baseType].get();
        }

        for(map<string, Class::Member>::iterator it2 = it->second->members.begin(); it2 != it->second->members.end(); it2++) {
            if(classes.find(it2->second.type) == classes.end())
                throw runtime_error("Undefined type " + it2->second.type.first + ":" + it2->second.type.second + " in member " + it2->first + " of " + it->first.first + ":" + it->first.second);

            it2->second.cl = classes[it2->second.type].get();
        }
    }
}

int main(int argc, char** argv) {
    if(argc <= 2) {
        printUsage();
        return 1;
    }

    XMLPlatformUtils::Initialize();

    //HACKHACK: we should handle NS lookup properly
    nsLUT["xs"] = XSL;
    nsLUT["xsl"] = XSL;
    nsLUT["xsd"] = XSL;

    addClass(shared_ptr<Class>(new IntClass));
    addClass(shared_ptr<Class>(new IntegerClass));
    addClass(shared_ptr<Class>(new LongClass));
    addClass(shared_ptr<Class>(new StringClass));
    addClass(shared_ptr<Class>(new AnyURIClass));
    addClass(shared_ptr<Class>(new FloatClass));
    addClass(shared_ptr<Class>(new DoubleClass));
    addClass(shared_ptr<Class>(new TimeClass));
    addClass(shared_ptr<Class>(new DateClass));
    addClass(shared_ptr<Class>(new DateTimeClass));
    addClass(shared_ptr<Class>(new BooleanClass));
    addClass(shared_ptr<Class>(new LanguageClass));

    string outputDir = argv[1];
    vector<string> schemaNames;

    for(int x = 2; x < argc; x++)
        schemaNames.push_back(argv[x]);

    work(outputDir, schemaNames);

    //dump the appenders and parsers of all non-build-in classes
    for(map<FullName, shared_ptr<Class> >::iterator it = classes.begin(); it != classes.end(); it++) {
        if(!it->second->isBuiltIn()) {
            if(!it->second->isSimple())
            {
                ostringstream oss;
                oss << outputDir << "/" << it->first.second << ".cpp";

                cout << oss.str() << endl;

                ofstream ofs(oss.str().c_str());

                it->second->writeImplementation(ofs);
            }

            {
                ostringstream oss;
                oss << outputDir << "/" << it->first.second << ".h";

                cout << oss.str() << endl;

                ofstream ofs(oss.str().c_str());

                it->second->writeHeader(ofs);
            }
        }
    }

    XMLPlatformUtils::Terminate();

    return 0;
}

