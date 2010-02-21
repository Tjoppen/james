/* 
 * File:   XMLObject.h
 * Author: tjoppen
 *
 * Created on February 14, 2010, 4:20 PM
 */

#ifndef _XMLOBJECT_H
#define	_XMLOBJECT_H

#include <xercesc/util/XercesDefs.hpp>
#include <ostream>

XERCES_CPP_NAMESPACE_BEGIN
    class DOMElement;
XERCES_CPP_NAMESPACE_END

namespace james {
    class XMLObject {
    public:
        XMLObject();
        virtual ~XMLObject();

        /**
         * Should create and append all member variables specified in the schema as DOM nodes to the specified target node.
         */
        virtual void appendChildren(xercesc::DOMElement *node) const = 0;

        /**
         * Should parse the values contained in the specified node into the member variables.
         */
        virtual void parseNode(xercesc::DOMElement *node) = 0;
    };
};

#endif	/* _XMLOBJECT_H */

