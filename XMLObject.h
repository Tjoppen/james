/* 
 * File:   XMLObject.h
 * Author: tjoppen
 *
 * Created on February 14, 2010, 4:20 PM
 */

#ifndef _XMLOBJECT_H
#define	_XMLOBJECT_H

#include <ostream>

namespace xercesc {
    class DOMNode;
}

namespace james {
    class XMLObject {
    public:
        XMLObject();
        virtual ~XMLObject();

    protected:
        /**
         * Should create and append all member variables specified in the schema as DOM nodes to the specified target node.
         */
        virtual void appendChildren(xercesc::DOMNode *node) const = 0;

        /**
         * Should parse the values contained in the specified node into the member variables.
         */
        virtual void parseNode(xercesc::DOMNode *node) = 0;
    };
};

#endif	/* _XMLOBJECT_H */

