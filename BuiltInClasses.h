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
    virtual std::string generateElementSetter(std::string memberName, std::string nodeName) const;
    virtual std::string generateAttributeSetter(std::string memberName, std::string attributeName) const;
    std::string generateParser() const;
    virtual std::string generateMemberSetter(std::string memberName, std::string nodeName) const;
    virtual std::string generateAttributeParser(std::string memberName, std::string attributeName) const;
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
    std::string getBaseHeader() const {
            return "<string>";
    }

    std::string generateElementSetter(std::string memberName, std::string nodeName) const {
        return nodeName + "->setTextContent(XercesString(" + memberName + "));";
    }

    std::string generateAttributeSetter(std::string memberName, std::string attributeName) const {
        return attributeName + "->setValue(XercesString(" + memberName + "));";
    }

    std::string generateMemberSetter(std::string memberName, std::string nodeName) const {
        return memberName + " = XercesString(" + nodeName + "->getTextContent());";
    }

    std::string generateAttributeParser(std::string memberName, std::string attributeName) const {
        return memberName + " = XercesString(" + attributeName + "->getValue());";
    }

    std::string getTester(std::string name) const {
        return name + ".length() > 0";
    }
};

GENERATE_BUILTIN(FloatClass, "float", "float", "0")};
GENERATE_BUILTIN(DoubleClass, "double", "double", "0")};

GENERATE_BUILTIN(BooleanClass, "boolean", "bool", "false")
    std::string generateElementSetter(std::string memberName, std::string nodeName) const {
        return nodeName + "->setTextContent(XercesString(" + memberName + " ? \"true\" : \"false\"));";
    }

    std::string generateAttributeSetter(std::string memberName, std::string attributeName) const {
        return attributeName + "->setValue(XercesString(" + memberName + " ? \"true\" : \"false\"));";
    }

    std::string generateMemberSetter(std::string memberName, std::string nodeName) const {
        std::ostringstream oss;

        oss << "{" << std::endl;
        oss << "//TODO: Strip string prior to this?" << std::endl;
        oss << "XercesString temp(" << nodeName << "->getTextContent());" << std::endl;
        oss << memberName << " = temp == \"true\" || temp == \"1\";" << std::endl;
        oss << "}" << std::endl;

        return oss.str();
    }

    std::string generateAttributeParser(std::string memberName, std::string attributeName) const {
        std::ostringstream oss;

        oss << "{" << std::endl;
        oss << "//TODO: Strip string prior to this?" << std::endl;
        oss << "XercesString temp(" << attributeName << "->getValue());" << std::endl;
        oss << memberName << " = temp == \"true\" || temp == \"1\";" << std::endl;
        oss << "}" << std::endl;

        return oss.str();
    }

    std::string getTester(std::string name) const {
        return name;
    }
};

//aliases
GENERATE_BUILTIN_ALIAS(IntegerClass, IntClass, "integer")};
GENERATE_BUILTIN_ALIAS(AnyURIClass, StringClass, "anyURI")};
GENERATE_BUILTIN_ALIAS(TimeClass, StringClass, "time")};
GENERATE_BUILTIN_ALIAS(DateClass, StringClass, "date")};
GENERATE_BUILTIN_ALIAS(DateTimeClass, StringClass, "dateTime")};
GENERATE_BUILTIN_ALIAS(LanguageClass, StringClass, "language")};

#endif	/* _BUILTINCLASSES_H */

