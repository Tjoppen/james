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
#include <limits.h>

#define UNBOUNDED INT_MAX

typedef std::string NamespaceName;
typedef std::string ClassName;
typedef std::pair<NamespaceName, ClassName> FullName;

class Class {
public:
    class Member {
    public:
        FullName type;
        Class *cl;
        int minOccurs;
        int maxOccurs;
        bool isAttribute;   //true if this member is an attribute rather than an element

        bool isArray() const;
        bool isRequired() const;

        std::string getType() const;
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

    bool hasBase;
    FullName baseType;
    Class *base;
    
    std::map<std::string, Member> members;

    Class(FullName name, ClassType type);
    Class(FullName name, ClassType type, FullName baseType);
    virtual ~Class();

    void addMember(std::string name, Member memberInfo);

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
     * Returns classname of base, or XMLObject if none.
     */
    std::string getBaseClassname() const;

    /**
     * Returns name of header wherein the base class is defined.
     */
    virtual std::string getBaseHeader() const;

    /**
     * Returns the base member type of this class.
     * Typically these fall into two categories:
     *  build-in simple types ("string" -> std::string, "int" -> int)
     *  complex types ("ExampleType" -> shared_ptr<ExampleType>)
     */
    std::string getClassType() const;

    /**
     * Should return a working default value with which to initialize an instance of the class.
     */
    virtual std::string getDefaultValue() const;

    /**
     * Should return a code fragment that tests whether the member of the specified name has been set.
     */
    virtual std::string getTester(std::string name) const;

    void writeImplementation(std::ostream& os) const;
    void writeHeader(std::ostream& os) const;
};

#endif	/* _CLASS_H */

