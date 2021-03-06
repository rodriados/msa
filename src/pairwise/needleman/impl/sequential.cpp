/**
 * Museqa: Multiple Sequence Aligner using hybrid parallel computing.
 * @file Sequential implementation for the pairwise module's needleman algorithm.
 * @author Rodrigo Siqueira <rodriados@gmail.com>
 * @copyright 2018-present Rodrigo Siqueira
 */
#include <cstdint>

#include "node.hpp"
#include "buffer.hpp"
#include "database.hpp"
#include "sequence.hpp"

#include "pairwise/pairwise.cuh"
#include "pairwise/needleman/needleman.cuh"

namespace
{
    using namespace museqa;
    using namespace pairwise;

    /**
     * Sequentially aligns two sequences using Needleman-Wunsch algorithm.
     * @param one The first sequence to align.
     * @param two The second sequence to align.
     * @param table The scoring table used to compare both sequences.
     * @return The alignment score.
     */
    static score align_pair(const sequence& one, const sequence& two, const scoring_table& table)
    {
        auto line = buffer<score>::make(two.length() + 1);

        // Filling 0-th line with penalties. This is the only initialization needed
        // for sequential algorithm.
        for(size_t i = 0; i <= two.length(); ++i)
            line[i] = i * -table.penalty();

        for(size_t i = 0; i < one.length(); ++i) {
            // If the current line is at sequence end, then we can already finish
            // the algorithm, as no changes are expected to occur after the end
            // of sequence.
            if(one[i] == sequence::padding)
                break;

            // Initialize the 0-th column values. It will always be initialized
            // with penalties, in the same manner as the 0-th line.
            score done = line[0];
            line[0] = (i + 1) * -table.penalty();

            // Iterate over the second sequence, calculating the best alignment
            // possible for each of its characters.
            for(size_t j = 1; j <= two.length(); ++j) {
                score value = line[j - 1];

                if(two[j - 1] != sequence::padding) {
                    const auto insertd = value - table.penalty();
                    const auto removed = line[j] - table.penalty();
                    const auto matched = done + table[{one[i], two[j - 1]}];
                    value = utils::max(matched, utils::max(insertd, removed));
                }

                done = line[j];
                line[j] = value;
            }
        }

        return line[two.length()];
    }

    /**
     * Executes the sequential Needleman-Wunsch algorithm for the pairwise step.
     * @param pairs The workpairs to align in the current node.
     * @param db The sequences available for alignment.
     * @param table The scoring table to use.
     * @return The score of aligned pairs.
     */
    static auto align(const buffer<pair>& pairs, const database& db, const scoring_table& table)
    -> buffer<score>
    {
        const size_t count = pairs.size();
        auto result = buffer<score>::make(count);

        for(size_t i = 0; i < count; ++i) {
            const sequence& one = db[pairs[i].first].contents;
            const sequence& two = db[pairs[i].second].contents;

            result[i] = align_pair(
                    one.size() > two.size() ? one : two
                ,   one.size() > two.size() ? two : one
                ,   table
                );
        }

        return result;
    }

    /**
     * The sequential needleman algorithm object. This algorithm uses no
     * parallelism, besides pairs distribution to run the Needleman-Wunsch
     * algorithm.
     * @since 0.1.1
     */
    struct sequential : public needleman::algorithm
    {
        /**
         * Executes the sequential needleman algorithm for the pairwise step. This method is
         * responsible for distributing and gathering workload from different cluster nodes.
         * @param context The algorithm's context.
         * @return The module's result value.
         */
        auto run(const context& ctx) const -> distance_matrix override
        {
            buffer<score> result;
            size_t nsequences = ctx.db.count();

            onlyslaves {
                auto pairs = this->generate(nsequences);
                result = align(pairs, ctx.db, ctx.table);
            }

            return distance_matrix {this->gather(result), nsequences};
        }
    };
}

namespace museqa
{
    /**
     * Instantiates a new sequential needleman instance.
     * @return The new algorithm instance.
     */
    extern auto pairwise::needleman::sequential() -> pairwise::algorithm *
    {
        return new ::sequential;
    }
}
