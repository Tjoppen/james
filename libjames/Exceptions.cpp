// This file is in the public domain.
#include "Exceptions.h"

using namespace james;
using namespace std;

Exception::Exception(const std::string& msg) throw() : exception(), msg(msg) {}

Exception::~Exception() throw() {}

const char* Exception::what() const throw() {
    return msg.c_str();
}

MissingRequiredElementException::MissingRequiredElementException(const std::string& msg) throw() : Exception(msg) {}

OutOfMemoryException::OutOfMemoryException(const std::string& msg) throw() : Exception(msg) {}

UnsetOptionalException::UnsetOptionalException(const std::string& msg) throw() : Exception(msg) {}
