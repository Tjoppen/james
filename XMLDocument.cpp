/* 
 * File:   XMLDocument.cpp
 * Author: tjoppen
 * 
 * Created on February 15, 2010, 1:22 PM
 */

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
#include <stdexcept>
#include <istream>
#include <ostream>
#include "XMLDocument.h"
#include "XMLObject.h"
#include "XercesString.h"

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

ostream& operator<< (ostream& os, const XMLDocument& doc) {
    //TODO: make exception safe
    const XMLObject         *object = dynamic_cast<const XMLObject*>(&doc);
    DOMImplementation       *implementation = DOMImplementationRegistry::getDOMImplementation(X("LS"));
    DOMImplementationLS     *lsImplementation = dynamic_cast<DOMImplementationLS*>(implementation);

    if(!object)             throw runtime_error(doc.getName() + " is not a sibling class of XMLObject");
    if(!implementation)     throw runtime_error("Failed to find a DOM implementation");
    if(!lsImplementation)   throw runtime_error("Failed to find a DOM LS implementation");

    DOMWriter *writer = lsImplementation->createDOMWriter();
    
    if(!writer)             throw runtime_error("Failed to create DOM writer");

    //get name of root element and create new DOM dodument
    DOMDocument *document = implementation->createDocument(0, X(doc.getName()), 0);

    if(!document)           throw runtime_error("Failed to create DOM document");

    DOMElement *root = document->getDocumentElement();

    if(!root)               throw runtime_error("Failed failed to get DOM document element");

    //append the nodes of each member variable to the root element in a recursive fashion
    object->appendChildren(root);

    //serialize
    XMLCh *str = writer->writeToString(*root);

    //convert XMLCh* string to std::string
    os << X(str);

    writer->release();
    document->release();

    //HACKHACK: if we don't do this valgrind complains
    //FIXME: we should call XMLString::release(), but that causes a mismatched delete[] - new thing
    delete str;

    return os;
}

istream& operator>> (istream& is, const XMLDocument& doc) {
    //parse XML, then parse objects from the resulting DOM tree
    XercesDOMParser parser;

    parser.setDoNamespaces(true);
    parser.parse(IStreamInputSource(is));

    const XMLObject *object = dynamic_cast<const XMLObject*>(&doc);
    DOMDocument     *document = parser.getDocument();
    DOMElement      *root = document->getDocumentElement();

    return is;
}
