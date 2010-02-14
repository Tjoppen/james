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
        FullName type;
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
    virtual std::string generateNodeSetter(std::string memberName, std::string nodeName) const;

    /**
     * Should return a code fragment that for parsing all the members of this Class.
     */
    virtual std::string generateParser() const;

    /**
     * Should return a code fragment that parses the value of a DOMElement into the named member.
     */
    virtual std::string generateMemberSetter(std::string memberName, std::string nodeName) const;

    /**
     * Should return the C++ class name of this Class.
     * Typically these fall into two categories:
     *  build-in simply types ("string" -> std::string, "int" -> int)
     *  complex types ("ExampleType" -> shared_ptr<ExampleType>)
     */
    virtual std::string getClassname() const;
};

#endif	/* _CLASS_H */

