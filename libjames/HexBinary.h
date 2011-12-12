/* This file is in the public domain.
 * 
 * File:   HexBinary.h
 * Author: tjoppen
 *
 * Created on April 16, 2010, 1:31 PM
 */

#ifndef _HEXBINARY_H
#define _HEXBINARY_H

#include <string>
#include <sstream>

namespace james {
    /**
     * Build-in class for dealing with xs:hexBinary.
     */
    class HexBinary {
        char *data;
        int size;

        void copyBuffer(const void *data, int size);

    public:
        HexBinary();
        HexBinary(const HexBinary& other);
        HexBinary(const std::string& str);
        HexBinary(const void *data, int size);
        ~HexBinary();

        void operator= (const HexBinary& other);
        void operator= (const std::string& str);
        void encode(std::ostream& os) const;
        void decode(std::istream& is);

        const void* getData() const;
        int getSize() const;
    };
}

/**
 * Encodes the given HexBinary into the given ostream.
 */
std::ostream& operator<< (std::ostream& os, const james::HexBinary& hex);

/**
 * Decodes hex from the given ostream into the given HexBinary, replacing any previous content.
 */
std::istream& operator>> (std::istream& is, james::HexBinary& hex);

#endif /* _HEXBINARY_H */

