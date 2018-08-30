/**
 * Multiple Sequence Alignment pairwise header file.
 * @author Rodrigo Siqueira <rodriados@gmail.com>
 * @copyright 2018 Rodrigo Siqueira
 */
#ifndef PW_PAIRWISE_HPP_INCLUDED
#define PW_PAIRWISE_HPP_INCLUDED

#pragma once

#include <cstdint>

#include "fasta.hpp"
#include "pairwise/sequence.cuh"

namespace pairwise
{
    /**
     * Indicates a pair of sequences to be aligned.
     * @since 0.1.alpha
     */
    struct Workpair
    {
        uint16_t first;     /// Index of the first sequence index to align
        uint16_t second;    /// Index of the second sequence index to align
    };

    /**
     * Stores score information about a sequence pair.
     * @since 0.1.alpha
     */
    struct Score
    {
        int32_t score = 0;          /// The cached score value for a sequence pair.
        uint16_t matches = 0;       /// The number of matches in the pair.
        uint16_t mismatches = 0;    /// The number of mismatches in the pair.
        uint16_t gaps = 0;          /// The number of gaps in the pair.
    };

    /**
     * Manages the pairwise module execution.
     * @since 0.1.alpha
     */
    class Pairwise
    {
        private:
            SequenceList list;
            Score *score = nullptr;
            uint32_t count = 0;

        public:
            Pairwise(const Fasta&);
            ~Pairwise() noexcept;

            /**
             * Informs the number of pairs processed or to process.
             * @return The number of pairs this instance shall process.
             */
            inline uint32_t getCount() const
            {
                return this->count;
            }

            /**
             * Gives access to the list of sequences to process.
             * @return The sequence list to process.
             */
            inline const SequenceList& getList() const
            {
                return this->list;
            }

            /**
             * Accesses a score according to its offset.
             * @return The requested pair score instance.
             */
            inline const Score& getScore(size_t offset) const
            {
                return this->score[offset];
            }

            /**
             * Gives access to a processed pair score instance.
             * @return The requested pair score instance.
             */
            inline const Score& getScore(uint16_t x, uint16_t y) const
            {
                uint16_t min = x > y ? y : x;
                uint16_t max = x > y ? x : y;

                return this->score[(max + 1) * max / 2 + min];
            }

            static Pairwise run(const Fasta&);

        friend class Needleman;
    };
};

#endif