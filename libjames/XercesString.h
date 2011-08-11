/* This file is in the public domain.
 * 
 * File:   XercesString.h
 * Author: tjoppen
 *
 * Created on February 12, 2010, 5:43 PM
 */

#ifndef _XERCESSTRING_H
#define	_XERCESSTRING_H

#include <xercesc/util/XercesDefs.hpp>
#include <string>
#include <ostream>

namespace james {
    /**
     * Wrapper class for dealing with XMLCh* strings.
     * Can convert from and to such strings.
     */
    class XercesString : public std::basic_string<XMLCh> {
    public:
        XercesString(const std::string& str);
        XercesString(const XMLCh *str);

        operator std::string () const;
        operator const XMLCh* () const;

        bool operator== (const std::string& str) const;
        bool operator!= (const std::string& str) const;
    };
}

std::ostream& operator<< (std::ostream& os, const james::XercesString& str);
std::ostream& operator<< (std::ostream& os, const XMLCh* str);

#endif	/* _XERCESSTRING_H */

