/* 
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
};

std::ostream& operator<< (std::ostream& os, const XercesString& str);

#endif	/* _XERCESSTRING_H */

