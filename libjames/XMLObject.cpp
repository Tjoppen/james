/* This file is in the public domain.
 * 
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
#ifdef USE_XERCES_C_28
#include <xercesc/dom/DOMWriter.hpp>
#else
#include <xercesc/dom/DOMLSSerializer.hpp>
#include <xercesc/dom/DOMLSOutput.hpp>
#endif
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

#ifdef USE_XERCES_C_28
        unsigned int curPos(void) const {
#else
        XMLFilePos curPos(void) const {
#endif
            return is.tellg();
        }

#ifdef USE_XERCES_C_28
        unsigned int readBytes(XMLByte* const buf, const unsigned int max) {
#else
        XMLSize_t readBytes(XMLByte* const buf, const XMLSize_t max) {
#endif
            is.read((char*)buf, max);

            return is.gcount();
        }

#ifndef USE_XERCES_C_28
        const XMLCh* getContentType() const {
            //TODO: return application/xml
            return NULL;
        }
#endif
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

#ifdef USE_XERCES_C_28
    void writeChars(const XMLByte* const toWrite, const unsigned int count, XMLFormatter* const formatter) {
#else
    void writeChars(const XMLByte* const toWrite, const XMLSize_t count, XMLFormatter* const formatter) {
#endif
        os.write((const char*)toWrite, count);
    }
};

//Generic deleter that calls T::release(). Works for a lot of classes in Xerces-C++
template<class T> class Releaser {
public:
    void operator () (T *t) {
        t->release();
    }
};

ostream& james::marshal(ostream& os, const XMLObject& obj, void (XMLObject::*appendChildren)(xercesc::DOMElement*) const, string documentName, string nameSpace) {
    XercesString ls("LS"), xmlns("xmlns");
    DOMImplementation       *implementation = DOMImplementationRegistry::getDOMImplementation(ls);
    DOMImplementationLS     *lsImplementation = dynamic_cast<DOMImplementationLS*>(implementation);

    if(!implementation)     throw runtime_error("Failed to find a DOM implementation");
    if(!lsImplementation)   throw runtime_error("Failed to find a DOM LS implementation");

    //shared_ptr + deleter -> exception safety
#ifdef USE_XERCES_C_28
    boost::shared_ptr<DOMWriter> writer(lsImplementation->createDOMWriter(), Releaser<DOMWriter>());

    if(!writer)             throw runtime_error("Failed to create DOM writer");
#else
    boost::shared_ptr<DOMLSSerializer> serializer(lsImplementation->createLSSerializer(), Releaser<DOMLSSerializer>());

    if(!serializer)         throw runtime_error("Failed to create DOM LS serializer");

    boost::shared_ptr<DOMLSOutput> output(lsImplementation->createLSOutput(), Releaser<DOMLSOutput>());

    if(!output)             throw runtime_error("Failed to create DOM LS output");
#endif

    //get name of root element and create new DOM dodument
    //shared_ptr + deleter -> exception safety
    XercesString documentNameString(documentName);
    boost::shared_ptr<DOMDocument> document(implementation->createDocument(0, documentNameString, 0), Releaser<DOMDocument>());

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

#ifdef USE_XERCES_C_28
    writer->writeNode(target.get(), *document);
#else
    output->setByteStream(target.get());
    serializer->write(document.get(), output.get());
#endif

    return os;
}

istream& james::unmarshal(istream& is, XMLObject& obj, void (XMLObject::*parseNode)(xercesc::DOMElement*), string name) {
    //parse XML, then parse objects from the resulting DOM tree
    XercesDOMParser parser;

    parser.setDoNamespaces(true);

    {
        IStreamInputSource iis(is);
        parser.parse(iis);
    }

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
