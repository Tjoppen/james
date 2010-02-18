/* 
 * File:   BuiltInClasses.h
 * Author: tjoppen
 *
 * Created on February 14, 2010, 6:48 PM
 */

#ifndef _BUILTINCLASSES_H
#define	_BUILTINCLASSES_H

#include "Class.h"

class BuiltInClass : public Class {
public:
    BuiltInClass(std::string name);
    virtual ~BuiltInClass();

    bool isBuiltIn() const;

    std::string generateAppender() const;
    std::string generateNodeSetter(std::string memberName, std::string nodeName) const;
    std::string generateParser() const;
    std::string generateMemberSetter(std::string memberName, std::string nodeName) const;
};

#define GENERATE_BUILTIN(name, xslName, classname, defaultValue)\
class name : public BuiltInClass {\
public:\
    name() : BuiltInClass(xslName) {}\
    std::string getClassname() const {return classname;}\
    std::string getDefaultValue() const {return defaultValue;}

GENERATE_BUILTIN(IntegerClass, "int", "int", "0")};
GENERATE_BUILTIN(LongClass, "long", "long", "0")};
GENERATE_BUILTIN(StringClass, "string", "std::string", "std::string()")
    std::string getTester(std::string name) const {
        return name + ".length() > 0";
    }
};

GENERATE_BUILTIN(AnyURIClass, "anyURI", "std::string", "std::string()")
    std::string getTester(std::string name) const {
        return name + ".length() > 0";
    }
};

#endif	/* _BUILTINCLASSES_H */

