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
#include <list>
#include <set>
#include <limits.h>

#define UNBOUNDED INT_MAX

typedef std::string NamespaceName;
typedef std::string ClassName;
typedef std::pair<NamespaceName, ClassName> FullName;

extern const std::string variablePostfix;

//commonly used temp variable names
extern const std::string nodeWithPostfix;       //"node" + variablePostfix
extern const std::string tempWithPostfix;       //"temp" + variablePostfix
extern const std::string convertedWithPostfix;  //"converted" + variablePostfix
extern const std::string ssWithPostfix;         //"ss" + variablePostfix

class Class {
public:
    class Member {
    public:
        std::string name;
        FullName type;
        Class *cl;
        int minOccurs;
        int maxOccurs;
        bool isAttribute;   //true if this member is an attribute rather than an element

        bool isArray() const;
        bool isOptional() const;    //returns true if this member is optional (not an array)
        bool isRequired() const;
    };

    enum ClassType {
        SIMPLE_TYPE,
        COMPLEX_TYPE,
    };

    bool isSimple() const;
    virtual bool isBuiltIn() const;

    const FullName name;
    const ClassType type;

    bool isDocument;            //true if this is a document class

    FullName baseType;
    Class *base;

    bool hasBase() const;
    
    std::list<Member> members;
    std::list<FullName> groups; //attributeGroups to add to this class

    Class(FullName name, ClassType type);
    Class(FullName name, ClassType type, FullName baseType);
    virtual ~Class();

    std::list<Member>::iterator findMember(std::string name);
    void addMember(Member memberInfo);

    /**
     * Should return a code fragment that for appending all the members of this Class.
     */
    virtual std::string generateAppender() const;

    /**
     * Should return a code fragment that sets the value/appends children to a DOMElement with the specified name.
     *
     * @param memberName The name of the member variable of
     * @param nodeName   The name of the DOMElement to set
     */
    virtual std::string generateElementSetter(std::string memberName, std::string nodeName) const;

    /**
     * Should return a code fragment that sets the node value of a DOMAttr to the string representation of the member the specified name.
     */
    virtual std::string generateAttributeSetter(std::string memberName, std::string attributeName) const;

    /**
     * Should return a code fragment that for parsing all the members of this Class.
     */
    virtual std::string generateParser() const;

    /**
     * Should return a code fragment that parses the value of a DOMElement into the named member.
     */
    virtual std::string generateMemberSetter(std::string memberName, std::string nodeName) const;

    /**
     * Should return a code fragment that parses the value of a DOMAttr into the named member.
     */
    virtual std::string generateAttributeParser(std::string memberName, std::string attributeName) const;

    /**
     * Should return the name with which to refer to this Class.
     */
    virtual std::string getClassname() const;

    /**
     * Returns name of header wherein the base class is defined.
     */
    virtual std::string getBaseHeader() const;

    std::set<std::string> getIncludedClasses() const;
    std::set<std::string> getPrototypeClasses() const;

    /**
     * Returns a list of the required elements of this Class.
     * Also includes those of its base if includingBase == true.
     */
    std::list<Member> getRequiredElements(bool includeBase) const;

    void writeImplementation(std::ostream& os) const;
    void writeHeader(std::ostream& os) const;
};

#endif	/* _CLASS_H */

