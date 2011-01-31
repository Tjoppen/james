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
    virtual std::string generateElementSetter(std::string memberName, std::string nodeName, std::string tabs) const;
    virtual std::string generateAttributeSetter(std::string memberName, std::string attributeName, std::string tabs) const;
    std::string generateParser() const;
    virtual std::string generateMemberSetter(std::string memberName, std::string nodeName, std::string tabs) const;
    virtual std::string generateAttributeParser(std::string memberName, std::string attributeName, std::string tabs) const;
};

#define GENERATE_BUILTIN(name, xslName, classname)\
class name : public BuiltInClass {\
public:\
    name() : BuiltInClass(xslName) {}\
    name(std::string xslOverride) : BuiltInClass(xslOverride) {}\
    std::string getClassname() const {return classname;}

//same as GENERATE_BUILTIN, except shouldUseConstReferences() is made to return false
#define GENERATE_BUILTIN_NONCONST(name, xslName, classname)\
GENERATE_BUILTIN(name, xslName, classname)\
    bool shouldUseConstReferences() const {return false;}

#define GENERATE_BUILTIN_ALIAS(name, base, override)\
class name : public base {\
public:\
    name() : base(override) {}

GENERATE_BUILTIN_NONCONST(ByteClass, "byte", "char")
    virtual std::string generateElementSetter(std::string memberName, std::string nodeName, std::string tabs) const;
    virtual std::string generateAttributeSetter(std::string memberName, std::string attributeName, std::string tabs) const;
    virtual std::string generateMemberSetter(std::string memberName, std::string nodeName, std::string tabs) const;
    virtual std::string generateAttributeParser(std::string memberName, std::string attributeName, std::string tabs) const;
};

GENERATE_BUILTIN_NONCONST(UnsignedByteClass, "unsignedByte", "unsigned char")
    virtual std::string generateElementSetter(std::string memberName, std::string nodeName, std::string tabs) const;
    virtual std::string generateAttributeSetter(std::string memberName, std::string attributeName, std::string tabs) const;
    virtual std::string generateMemberSetter(std::string memberName, std::string nodeName, std::string tabs) const;
    virtual std::string generateAttributeParser(std::string memberName, std::string attributeName, std::string tabs) const;
};


GENERATE_BUILTIN_NONCONST(ShortClass, "short", "short")};
GENERATE_BUILTIN_NONCONST(UnsignedShortClass, "unsignedShort", "unsigned short")};
GENERATE_BUILTIN_NONCONST(IntClass, "int", "int")};
GENERATE_BUILTIN_NONCONST(UnsignedIntClass, "unsignedInt", "unsigned int")};
GENERATE_BUILTIN_NONCONST(LongClass, "long", "long long")};
GENERATE_BUILTIN_NONCONST(UnsignedLongClass, "unsignedLong", "unsigned long long")};
GENERATE_BUILTIN(StringClass, "string", "std::string")
    std::string getBaseHeader() const {
            return "<string>";
    }

    std::string generateElementSetter(std::string memberName, std::string nodeName, std::string tabs) const {
        return tabs + "{ XercesString " + tempWithPostfix + "(" + memberName + "); " + nodeName + "->setTextContent(" + tempWithPostfix + "); }";
    }

    std::string generateAttributeSetter(std::string memberName, std::string attributeName, std::string tabs) const {
        return tabs + "{ XercesString " + tempWithPostfix + "(" + memberName + "); " + attributeName + "->setValue(" + tempWithPostfix + "); }";
    }

    std::string generateMemberSetter(std::string memberName, std::string nodeName, std::string tabs) const {
        return tabs + memberName + " = XercesString(" + nodeName + "->getTextContent());";
    }

    std::string generateAttributeParser(std::string memberName, std::string attributeName, std::string tabs) const {
        return tabs + memberName + " = XercesString(" + attributeName + "->getValue());";
    }
};

GENERATE_BUILTIN_NONCONST(FloatClass, "float", "float")};
GENERATE_BUILTIN_NONCONST(DoubleClass, "double", "double")};

GENERATE_BUILTIN_NONCONST(BooleanClass, "boolean", "bool")
    std::string generateElementSetter(std::string memberName, std::string nodeName, std::string tabs) const {
        return tabs + "{ XercesString " + tempWithPostfix + "(" + memberName + " ? \"true\" : \"false\"); " + nodeName + "->setTextContent(" + tempWithPostfix + "); }";
    }

    std::string generateAttributeSetter(std::string memberName, std::string attributeName, std::string tabs) const {
        return tabs + "{ XercesString " + tempWithPostfix + "(" + memberName + " ? \"true\" : \"false\"); " + attributeName + "->setValue(" + tempWithPostfix + "); }";
    }

    std::string generateMemberSetter(std::string memberName, std::string nodeName, std::string tabs) const {
        std::ostringstream oss;

        oss << tabs << "{" << std::endl;
        oss << tabs << "//TODO: Strip string prior to this?" << std::endl;
        oss << tabs << "XercesString " << tempWithPostfix << "(" << nodeName << "->getTextContent());" << std::endl;
        oss << tabs << memberName << " = " << tempWithPostfix << " == \"true\" || " << tempWithPostfix << " == \"1\";" << std::endl;
        oss << tabs << "}" << std::endl;

        return oss.str();
    }

    std::string generateAttributeParser(std::string memberName, std::string attributeName, std::string tabs) const {
        std::ostringstream oss;

        oss << tabs << "{" << std::endl;
        oss << tabs << "//TODO: Strip string prior to this?" << std::endl;
        oss << tabs << "XercesString " << tempWithPostfix << "(" << attributeName << "->getValue());" << std::endl;
        oss << tabs << memberName << " = " << tempWithPostfix << " == \"true\" || " << tempWithPostfix << " == \"1\";" << std::endl;
        oss << tabs << "}" << std::endl;

        return oss.str();
    }
};

GENERATE_BUILTIN(HexBinaryClass, "hexBinary", "james::HexBinary")};

//aliases
GENERATE_BUILTIN_ALIAS(IntegerClass, IntClass, "integer")};
GENERATE_BUILTIN_ALIAS(AnyURIClass, StringClass, "anyURI")};
GENERATE_BUILTIN_ALIAS(TimeClass, StringClass, "time")};
GENERATE_BUILTIN_ALIAS(DateClass, StringClass, "date")};
GENERATE_BUILTIN_ALIAS(DateTimeClass, StringClass, "dateTime")};
GENERATE_BUILTIN_ALIAS(LanguageClass, StringClass, "language")};

#endif	/* _BUILTINCLASSES_H */

