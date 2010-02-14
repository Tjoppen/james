/* 
 * File:   IntegerClass.h
 * Author: tjoppen
 *
 * Created on February 14, 2010, 6:48 PM
 */

#ifndef _INTEGERCLASS_H
#define	_INTEGERCLASS_H

#include "Class.h"

class IntegerClass : public Class {
public:
    IntegerClass();
    virtual ~IntegerClass();

    std::string generateAppender() const;
    std::string generateNodeSetter(std::string memberName, std::string nodeName) const;
    std::string generateParser() const;
    std::string generateMemberSetter(std::string memberName, std::string nodeName) const;
    std::string getClassname() const;
};

#endif	/* _INTEGERCLASS_H */

