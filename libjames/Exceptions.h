// This file is in the public domain.
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>

namespace james {
    /**
     * Base class for all exceptions thrown during marshalling and unmarshalling.
     */
    class Exception : public std::exception {
        std::string msg;
    public:
        Exception(const std::string& msg) throw();
        ~Exception() throw();

        const char* what() const throw();
    };

    //thrown when a required element of a complex type is NULL
    class MissingRequiredElementException : public Exception {
    public:
        MissingRequiredElementException(const std::string& msg) throw();
    };

    //thrown when a memory allocation failed
    class OutOfMemoryException : public Exception {
    public:
        OutOfMemoryException(const std::string& msg) throw();
    };

    //thrown when the user attempts to access the value of an unset james::optional
    class UnsetOptionalException : public Exception {
    public:
        UnsetOptionalException(const std::string& msg) throw();
    };
};

#endif // EXCEPTIONS_H
