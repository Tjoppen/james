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
    virtual std::string generateNodeSetter(std::string memberName, std::string nodeName) const;
    std::string generateParser() const;
    std::string generateMemberSetter(std::string memberName, std::string nodeName) const;
};

#define GENERATE_BUILTIN(name, xslName, classname, defaultValue)\
class name : public BuiltInClass {\
public:\
    name() : BuiltInClass(xslName) {}\
    name(std::string xslOverride) : BuiltInClass(xslOverride) {}\
    std::string getClassname() const {return classname;}\
    std::string getDefaultValue() const {return defaultValue;}

#define GENERATE_BUILTIN_ALIAS(name, base, override)\
class name : public base {\
public:\
    name() : base(override) {}

GENERATE_BUILTIN(IntClass, "int", "int", "0")};
GENERATE_BUILTIN(LongClass, "long", "long", "0")};
GENERATE_BUILTIN(StringClass, "string", "std::string", "std::string()")
    std::string generateNodeSetter(std::string memberName, std::string nodeName) const {
        return nodeName + "->setTextContent(XercesString(" + memberName + "));";
    }

    std::string generateMemberSetter(std::string memberName, std::string nodeName) const {
        return memberName + " = XercesString(" + nodeName + "->getTextContent());";
    }

    std::string getTester(std::string name) const {
        return name + ".length() > 0";
    }
};

//aliases
GENERATE_BUILTIN_ALIAS(IntegerClass, IntClass, "integer")};
GENERATE_BUILTIN_ALIAS(AnyURIClass, StringClass, "anyURI")};

#endif	/* _BUILTINCLASSES_H */

