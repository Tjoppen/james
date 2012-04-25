/* This file is in the public domain.
 * 
 * File:   optional.h
 * Author: tjoppen
 *
 * Created on December 13, 2010, 4:44 PM
 */

#ifndef _OPTIONAL_H
#define _OPTIONAL_H

#include "Exceptions.h"

#ifdef __GNUC__
#if (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#define james_attribute_deprecated __attribute__((deprecated))
#else
#define james_attribute_deprecated
#endif
#else
#define james_attribute_deprecated
#endif

namespace james {
    /**
     * Like boost::optional, except with pointer storage.
     *
     * Note: This may be quite slow for built-in types like int. Consider using
     *       boost::optional in that case.
     */
    template<class T> class optional {
        T *t;

        void assign(const T *rhs) {
            if(!rhs) {
                if(t) {
                    delete t;
                    t = NULL;
                }

                return;
            }

            T *newT = new T(*rhs);

            if(t)
                delete t;

            t = newT;
        }

        void throwIfNotSet() const {
            if(!isSet())
                throw UnsetOptionalException("Tried to access optional value that isn't set");
        }
    public:
        optional() : t(NULL) {
        }

        optional(const T& t) : t(NULL) {
            assign(&t);
        }

        optional(const optional& other) : t(NULL) {
            assign(other.t);
        }

        ~optional() {
            if(t)
                delete t;
        }

        bool isSet() const {
            return t != NULL;
        }

        optional& operator = (const T& rhs) {
            assign(&rhs);
            return *this;
        }

        optional& operator = (const optional& rhs) {
            assign(rhs.t);
            return *this;
        }

        const T* operator -> () const {
            throwIfNotSet();

            return t;
        }

        T* operator -> () {
            throwIfNotSet();

            return t;
        }

        const T& get() const {
            throwIfNotSet();

            return *t;
        }

        T& get() {
            throwIfNotSet();

            return *t;
        }

        /**
         * Like james::optional::get() except it takes a default value that is
         * returned if !isSet().
         * This is useful for avoiding overly long lines like:
         * 
         *  fooValue = fooElement.isSet() ? fooElement.get() : defaultValue;
         * 
         * Intead this function can be used like this:
         * 
         *  fooValue = fooElement.getOrDefault(defaultValue);
         */
        const T& getOrDefault(const T& defaultValue) const {
            if(!isSet())
                return defaultValue;
            else
                return *t;
        }

        T& getOrDefault(T& defaultValue) {
            if(!isSet())
                return defaultValue;
            else
                return *t;
        }

        void clear() {
            assign(NULL);
        }
    };
}

#endif /* _OPTIONAL_H */

