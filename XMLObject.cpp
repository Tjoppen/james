/* 
 * File:   XMLObject.cpp
 * Author: tjoppen
 * 
 * Created on March 19, 2010, 10:44 AM
 */

#include "XMLObject.h"
#include "XercesString.h"
#include <iostream>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

using namespace xercesc;
using namespace james;
using namespace std;

/**
 * Minimal utility class for parsing directly from an std::istream.
 */
class IStreamInputSource : public InputSource {
    istream &is;

    class IStreamBinInputStream : public BinInputStream {
        istream &is;
    public:
        IStreamBinInputStream(istream &is) : BinInputStream(), is(is) {
        }

        unsigned int curPos(void) const {
            return is.tellg();
        }

        unsigned int readBytes(XMLByte* const buf, const unsigned int max) {
            is.read((char*)buf, max);

            return is.gcount();
        }
    };
public:
    IStreamInputSource(istream &is) : InputSource(), is(is) {
    }

    BinInputStream* makeStream() const {
        return new IStreamBinInputStream(is);
    }
};

/**
 * Used for serializing directly to an std::ostream.
 */
class OstreamFormatTarget : public XMLFormatTarget {
    ostream& os;

public:
    OstreamFormatTarget(ostream& os) : os(os) {
    }

    void writeChars(const XMLByte* const toWrite, const unsigned int count, XMLFormatter* const formatter) {
        os.write((const char*)toWrite, count);
    }
};

//deleters
class WriterDeleter {
public:
    void operator () (DOMWriter *writer) {
        writer->release();
    }
};

class DocumentDeleter {
public:
    void operator () (DOMDocument *document) {
        document->release();
    }
};

ostream& james::marshal(ostream& os, const XMLObject& obj, void (XMLObject::*appendChildren)(xercesc::DOMElement*) const, string documentName, string nameSpace) {
    XercesString ls("LS"), xmlns("xmlns");
    DOMImplementation       *implementation = DOMImplementationRegistry::getDOMImplementation(ls);
    DOMImplementationLS     *lsImplementation = dynamic_cast<DOMImplementationLS*>(implementation);

    if(!implementation)     throw runtime_error("Failed to find a DOM implementation");
    if(!lsImplementation)   throw runtime_error("Failed to find a DOM LS implementation");

    //shared_ptr + deleter -> exception safety
    boost::shared_ptr<DOMWriter> writer(lsImplementation->createDOMWriter(), WriterDeleter());

    if(!writer)             throw runtime_error("Failed to create DOM writer");

    //get name of root element and create new DOM dodument
    //shared_ptr + deleter -> exception safety
    XercesString documentNameString(documentName);
    boost::shared_ptr<DOMDocument> document(implementation->createDocument(0, documentNameString, 0), DocumentDeleter());

    if(!document)           throw runtime_error("Failed to create DOM document");

    DOMElement *root = document->getDocumentElement();

    if(!root)               throw runtime_error("Failed failed to get DOM document element");

    XercesString nameSpaceString(nameSpace);
    root->setAttribute(xmlns, nameSpaceString);

    //append the nodes of each member variable to the root element in a recursive fashion
    (obj.*appendChildren)(root);

    //serialize directly to the ostream
    //we only need a scoped_ptr here
    boost::scoped_ptr<OstreamFormatTarget> target(new OstreamFormatTarget(os));

    writer->writeNode(target.get(), *document);

    return os;
}

istream& james::unmarshal(istream& is, XMLObject& obj, void (XMLObject::*parseNode)(xercesc::DOMElement*), string name) {
    //parse XML, then parse objects from the resulting DOM tree
    XercesDOMParser parser;

    parser.setDoNamespaces(true);
    parser.parse(IStreamInputSource(is));

    DOMDocument     *document = parser.getDocument();

    if(!document)   throw runtime_error("Failed to parse document");

    DOMElement      *root = document->getDocumentElement();

    if(!root)       throw runtime_error("Failed to get document root");

    //check that the name of the root node matches the name of the XMLDocument
    if(!root->getLocalName())
        throw runtime_error("No local name for DOM document (expected " + name + ")");

    XercesString rootName(root->getLocalName());
    if(rootName != name)
        throw runtime_error("Supplied XML (" + (string)rootName + ") is not a " + name);

    (obj.*parseNode)(root);

    return is;
}
