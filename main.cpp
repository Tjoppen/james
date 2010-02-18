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

static string lookupNamespace(string typeName) {
    //figures out namespace URI of given type
    size_t pos = typeName.find_last_of(':');

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

static FullName toFullName(string typeName) {
    //looks up and strips namespace from typeName and builds a FullName of the result
    return FullName(lookupNamespace(typeName), stripNamespace(typeName));
}

static void parseComplexType(DOMElement *element, FullName fullName);

static void parseSequence(DOMElement *parent, DOMElement *sequence, shared_ptr<Class> cl) {
    //we expect to see a whole bunch of <element>s here
    CHECK(parent);
    CHECK(sequence);
    
    for(DOMNode *child = sequence->getFirstChild(); child; child = child->getNextSibling())
        if(child->getNodeType() == DOMNode::ELEMENT_NODE && X(child->getLocalName()) == "element") {
            DOMElement *childElement = dynamic_cast<DOMElement*>(child);
            CHECK(childElement);

            int minOccurs = 1;
            int maxOccurs = 1;

            XercesString typeStr("type");
            XercesString minOccursStr("minOccurs");
            XercesString maxOccursStr("maxOccurs");
            XercesString name = childElement->getAttribute(X("name"));

            if(childElement->hasAttribute(minOccursStr)) {
                stringstream ss;
                ss << X(childElement->getAttribute(minOccursStr));
                ss >> minOccurs;
            }

            if(childElement->hasAttribute(maxOccursStr)) {
                XercesString str(childElement->getAttribute(maxOccursStr));

                if(str == "unbounded")
                    maxOccurs = UNBOUNDED;
                else {
                    stringstream ss;
                    ss << str;
                    ss >> maxOccurs;
                }
            }

            if(childElement->hasAttribute(typeStr)) {
                //has type == end point - add as member of cl
                Class::Member info;
                
                //assume in same namespace for now
                info.type = toFullName(X(childElement->getAttribute(typeStr)));
                info.minOccurs = minOccurs;
                info.maxOccurs = maxOccurs;
                info.isAttribute = false;

                cl->addMember(name, info);
            } else {
                //no type - anonymous subtype
                //generate name
                FullName subName(cl->name.first, cl->name.second + "_" + (string)name);

                //expect <complexType> sub-tag
                DOMNode *child2;
                for(child2 = child->getFirstChild(); child2; child2 = child2->getNextSibling())
                    if(child2->getNodeType() == DOMNode::ELEMENT_NODE && child2->getLocalName() && X(child2->getLocalName()) == "complexType") {
                        parseComplexType(dynamic_cast<DOMElement*>(child2), subName);
                        break;
                    }

                if(!child2)
                    throw runtime_error("Missing <complexType> in anonymous type in " + cl->name.second);

                Class::Member info;
                info.type = subName;
                info.minOccurs = minOccurs;
                info.maxOccurs = maxOccurs;
                info.isAttribute = false;

                cl->addMember(name, info);
            }
        }
}

static void parseComplexType(DOMElement *element, FullName fullName) {
    //we handle two cases with <complexType>:
    //child is <sequence>
    //child is <complexContent> - expect grandchild <extension>
    CHECK(element);

    for(DOMNode *child = element->getFirstChild(); child; child = child->getNextSibling()) {
        if(child->getNodeType() == DOMNode::ELEMENT_NODE) {
            XercesString name(child->getLocalName());

            if(name == "sequence") {
                //TODO: Add new Class based on the name of element
                shared_ptr<Class> cl = addClass(shared_ptr<Class>(new Class(fullName, Class::COMPLEX_TYPE)));

                parseSequence(element, dynamic_cast<DOMElement*>(child), cl);
            } else if(name == "complexContent") {
                throw runtime_error("complexContent not currently supported");
            } else {
                throw runtime_error("Unknown complexType child of type " + (string)name);
            }
        }
    }
}

static void parseSimpleType(DOMElement *element, FullName fullName) {
    //expect a <restriction> child element
    CHECK(element);

    for(DOMNode *child = element->getFirstChild(); child; child = child->getNextSibling())
        if(child->getNodeType() == DOMNode::ELEMENT_NODE && child->getLocalName() && X(child->getLocalName()) == "restriction") {
            DOMElement *childElement = dynamic_cast<DOMElement*>(child);
            CHECK(childElement);

            if(!childElement->hasAttribute(X("base")))
                throw runtime_error("simpleType restriction lacks expected attribute 'base'");

            //convert xs:string and the like to their respective FullName
            FullName baseName = toFullName(X(childElement->getAttribute(X("base"))));

            //add class and return
            addClass(shared_ptr<Class>(new Class(fullName, Class::SIMPLE_TYPE, baseName)));
            return;
        }

    throw runtime_error("simpleType expected restriction");
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
        //return; //ignore element for now
        if(!element->hasAttribute(X("type")))
            throw runtime_error("Missing type on <element>");

        XercesString type(element->getAttribute(X("type")));

        cout << name << " of type " << type << endl;

        addClass(shared_ptr<Class>(new Class(fullName, Class::COMPLEX_TYPE, FullName(tns, type))))->isDocument = true;
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

        for(DOMNode *child = root->getFirstChild(); child; child = child->getNextSibling())
            if(child->getNodeType() == DOMNode::ELEMENT_NODE)
                parseElement(dynamic_cast<DOMElement*>(child), tns);
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

