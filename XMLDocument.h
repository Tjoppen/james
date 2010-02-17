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
         * Should return the name of the root node in this document.
         */
        virtual std::string getName() const = 0;
    };
}

std::ostream& operator<< (std::ostream& os, const james::XMLDocument& doc);
std::istream& operator>> (std::istream& is, const james::XMLDocument& doc);

#endif	/* _XMLDOCUMENT_H */

