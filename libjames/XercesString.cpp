/* 
 * File:   XercesString.cpp
 * Author: tjoppen
 * 
 * Created on February 12, 2010, 5:43 PM
 */

#include "XercesString.h"
#include <xercesc/util/XMLString.hpp>
#include <boost/shared_ptr.hpp>

using namespace xercesc;
using namespace boost;
using namespace std;
using namespace james;

class XMLChDeleter {
public:
    void operator() (XMLCh *str) {
        XMLString::release(&str);
    }
};

XercesString::XercesString(const string& str) {
    shared_ptr<XMLCh> xstr(XMLString::transcode(str.c_str()), XMLChDeleter());

    *this = xstr.get();
}

XercesString::XercesString(const XMLCh *str) : basic_string<XMLCh>(str) {
}

XercesString::operator string () const {
    char *str = XMLString::transcode(c_str());

    string ret = str;

    XMLString::release(&str);

    return ret;
}

XercesString::operator const XMLCh* () const {
    return c_str();
}

bool XercesString::operator== (const string& str) const {
    return (string)*this == str;
}

bool XercesString::operator!= (const string& str) const {
    return !(*this == str);
}

ostream& operator<< (ostream& os, const XercesString& str) {
    return os << (string)str;
}

ostream& operator<< (ostream& os, const XMLCh* str) {
    if(!str)
        return os << "(null)";
    
    return os << XercesString(str);
}
