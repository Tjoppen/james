/* 
 * File:   Class.h
 * Author: tjoppen
 *
 * Created on February 14, 2010, 4:20 PM
 */

#ifndef _CLASS_H
#define	_CLASS_H

#include <string>
#include <map>

#define UNBOUNDED INT_MAX

typedef std::string NamespaceName;
typedef std::string ClassName;
typedef std::pair<NamespaceName, ClassName> FullName;

class Class {
public:
    class Member {
    public:
        Class *cl;
        int minOccurs;
        int maxOccurs;
    };

    enum ClassType {
        SIMPLE_TYPE,
        COMPLEX_TYPE,
    };

    FullName name;

    Class *base;

    ClassType type;
    
    std::map<std::string, Member> members;

    Class(FullName name, ClassType type);
    Class(FullName name, ClassType type, Class *base);
    virtual ~Class();
};

#endif	/* _CLASS_H */

