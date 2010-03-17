/* 
 * File:   XMLDocument.h
 * Author: tjoppen
 *
 * Created on February 15, 2010, 1:22 PM
 */

#ifndef _XMLDOCUMENT_H
#define	_XMLDOCUMENT_H

#include <string>

namespace james {
    /**
     * An XMLDocument is an interface for an XMLObject which is the root of a document.
     * In addition to having children, like a normal XMLObject in accordance
     * with the schema, it also has a name.
     */
    class XMLDocument {
    public:
        /**
         * Should return the namespace URI of this document.
         */
        virtual std::string getNamespace() const = 0;

        /**
         * Should return the name of the root node in this document.
         */
        virtual std::string getName() const = 0;

        /**
         * Should create and append all member variables specified in the schema as DOM nodes to the specified target node.
         */
        virtual void appendChildren(xercesc::DOMElement *node) const = 0;

        /**
         * Should parse the values contained in the specified node into the member variables.
         */
        virtual void parseNode(xercesc::DOMElement *node) = 0;
    };
}

std::ostream& operator<< (std::ostream& os, const james::XMLDocument& doc);
std::istream& operator>> (std::istream& is, james::XMLDocument& doc);

#endif	/* _XMLDOCUMENT_H */

