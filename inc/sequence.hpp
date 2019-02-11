/** 
 * Multiple Sequence Alignment sequence header file.
 * @author Rodrigo Siqueira <rodriados@gmail.com>
 * @copyright 2018-2019 Rodrigo Siqueira
 */
#pragma once

#ifndef SEQUENCE_HPP_INCLUDED
#define SEQUENCE_HPP_INCLUDED

#include <cstdint>
#include <ostream>
#include <string>

#include "buffer.hpp"

/**
 * Creates an sequence. This sequence is a buffer an any modification to
 * it shall be implemented by inherited methods.
 * @since 0.1.alpha
 */
class Sequence : public Buffer<char>
{
    public:
        Sequence() = default;
        Sequence(const Sequence&) = default;
        Sequence(Sequence&&) = default;
        
        using Buffer<char>::Buffer;

        /**
         * Instantiates a new sequence.
         * @param string The string containing this sequence's data.
         */
        inline Sequence(const std::string& string)
        :   Buffer<char> {string.c_str(), string.size()}
        {}

        Sequence& operator=(const Sequence&) = default;
        Sequence& operator=(Sequence&&) = default;

        /**
         * Informs the length of the sequence.
         * @return The sequence's length.
         */
        inline size_t getLength() const
        {
            return this->getSize();
        }

        /**
         * Transforms the sequence into a string.
         * @return The sequence representation as a string.
         */
        inline std::string toString() const
        {
            return {this->getBuffer(), this->getLength()};
        }
};

/**
 * This function allows buffers to be directly printed into an ostream instance.
 * @param os The output stream instance.
 * @param sequence The sequence to print.
 */
inline std::ostream& operator<<(std::ostream& os, const BaseBuffer<char>& sequence)
{
    os << std::string(sequence.getBuffer(), sequence.getSize());
    return os;
}

#endif