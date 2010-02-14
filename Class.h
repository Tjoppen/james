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

class Class {
public:
    class Member {
    public:
        Class *cl;
        int minOccurs;
        int maxOccurs;
    };

    std::string ns;
    std::string name;
    std::map<std::string, Member> members;

    Class(std::string ns, std::string name);
    virtual ~Class();
};

#endif	/* _CLASS_H */

