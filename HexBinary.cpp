/* 
 * File:   HexBinary.cpp
 * Author: tjoppen
 * 
 * Created on April 16, 2010, 1:31 PM
 */

#include <memory.h>
#include "HexBinary.h"
#include "XMLObject.h"

using namespace james;

HexBinary::HexBinary() : data(NULL) {}

HexBinary::HexBinary(const std::string& str) : data(NULL) {
    copyBuffer(str.c_str(), str.size());
}

HexBinary::HexBinary(const char *data, int size) : data(NULL) {
    copyBuffer(data, size);
}

void HexBinary::operator= (const std::string& str) {
    copyBuffer(str.c_str(), str.size());
}

HexBinary::~HexBinary() {
    if(data)
        delete [] data;
}

void HexBinary::copyBuffer(const char *inData, int inSize) {
    if(data) {
        delete [] data;
        data = NULL;
    }

    size = inSize;
    
    if(!(data = new char [size]))
        throw OutOfMemoryException("Failed to allocate HexBinary::data");

    memcpy(data, inData, size);
}

void HexBinary::encode(std::ostream& os) const {
    const char lut[] = "0123456789ABCDEF";

    for(int x = 0; x < size; x++) {
        os << lut[(data[x] >> 4) & 0xF];
        os << lut[data[x] & 0xF];
    }
}

void HexBinary::decode(std::istream& is) {
    std::ostringstream oss;

    for(;;) {
        int shift = 4;
        char hex = 0;

        while(shift >= 0) {
            char c;

            if(!(is >> c))
                goto done;

            if(c >= '0' && c <= '9') {
                hex |= (c - '0') << shift;
                shift -= 4;
            } else if(c >= 'A' && c <= 'F') {
                hex |= (c - 'A' + 10) << shift;
                shift -= 4;
            } else if(c >= 'a' && c <= 'f') {
                hex |= (c - 'a' + 10) << shift;
                shift -= 4;
            }
        }

        oss << hex;
    }
    
done:
    copyBuffer(oss.str().c_str(), oss.str().size());
}

const void* HexBinary::getData() const {
    return data;
}

int HexBinary::getSize() const {
    return size;
}

std::ostream& operator<< (std::ostream& os, const HexBinary& hex) {
    hex.encode(os);
    return os;
}

std::istream& operator>> (std::istream& is, HexBinary& hex) {
    hex.decode(is);
    return is;
}
