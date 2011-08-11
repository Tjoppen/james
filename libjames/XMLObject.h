/* This file is in the public domain.
 * 
 * File:   XMLObject.h
 * Author: tjoppen
 *
 * Created on March 19, 2010, 10:44 AM
 */

#ifndef _XMLOBJECT_H
#define	_XMLOBJECT_H

#include <string>
#include <xercesc/util/XercesDefs.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>

XERCES_CPP_NAMESPACE_BEGIN
    class DOMElement;
XERCES_CPP_NAMESPACE_END

namespace james {
    /**
     * XMLObject
     *
     * Base class of all generated XML objects. Derived classes are required/guaranteed to fulfill the following contract:
     * 
     *  Implement the following methods:
     *
     *      Name: getName
     *   Purpose: Should return the class name of the object
     *      Form: std::string getName() const
     *
     *      Name: getNamespace
     *   Purpose: Should return the namespace URI of the object
     *      Form: std::string getNamespace() const
     *
     *      Name: appendChildren
     *   Purpose: Should create and append all member variables specified in the schema as DOM nodes to the specified target node
     *      Form: void appendChildren(xercesc::DOMElement*) const
     *
     *      Name: parseNode
     *   Purpose: Should parse the values contained in the specified node into the member variables
     *      Form: void parseNode(xercesc::DOMElement*)
     */
    class XMLObject {};

    /**
     * Internal utility function for marshalling XMLObjects.
     *
     * throws: MissingRequiredElementException if one or more required complex elements is missing
     */
    std::ostream& marshal(std::ostream& os, const james::XMLObject& obj, void (james::XMLObject::*appendChildren)(xercesc::DOMElement*) const, std::string name, std::string nameSpace);

    /**
     * Internal utility function for unmarshalling XMLObjects.
     */
    std::istream& unmarshal(std::istream& is, james::XMLObject& obj, void (james::XMLObject::*parseNode)(xercesc::DOMElement*), std::string name);
}

/**
 * Templated implementation of ostream::operator<<() for all XMLObjects.
 * Marhsals the given object of type T into the given ostream.
 */
template<typename T>
        typename boost::enable_if<boost::is_base_of<james::XMLObject, T>, std::ostream&>::type
        operator<< (std::ostream& os, const T& obj) {
    return james::marshal(os, obj, static_cast<void (james::XMLObject::*)(xercesc::DOMElement*) const>(&T::appendChildren), obj.getName(), obj.getNamespace());
}

/**
 * Templated implementation of istream::operator>>() for all XMLObjects.
 * Unmarshals an object of type T from the given istream into obj.
 */
template<typename T>
        typename boost::enable_if<boost::is_base_of<james::XMLObject, T>, std::istream&>::type
        operator>> (std::istream& is, T& obj) {
    return james::unmarshal(is, obj, static_cast<void (james::XMLObject::*)(xercesc::DOMElement*)>(&T::parseNode), obj.getName());
}

#endif	/* _XMLOBJECT_H */

