/** @file pairwise.cuh
 * @brief Parallel Multiple Sequence Alignment pairwise header file.
 * @author Rodrigo Siqueira <rodriados@gmail.com>
 * @copyright 2018 Rodrigo Siqueira
 */
#ifndef _PAIRWISE_CUH
#define _PAIRWISE_CUH

#include <cstdint>
#include <vector>

#include "fasta.hpp"

#define __msa_threads_per_block__ 32
//#define __msa_prefer_shared_mem__ 1

/** @struct position_t
 * @brief Informs how to access a sequence from a continuous char-pointer.
 * @var offset Indicates the sequence offset to the pointer.
 * @var length Indicates how big is the sequence.
 */
typedef struct {
    uint32_t offset;
    uint32_t length;
} position_t;

/** @struct workpair_t
 * @brief Indicates a pair of sequences to be aligned.
 * @var seq The pair of sequences to align.
 */
typedef struct {
    uint16_t seq[2];
} workpair_t;

/** @struct score_t
 * @brief Stores score information about a sequence pair.
 * @var cached The cached score value for a sequence pair.
 * @var matches The number of matches in the pair.
 * @var mismatches The number of mismatches in the pair.
 * @var gaps The number of gaps in the pair.
 */
typedef struct {
    int32_t cached;
    uint16_t matches;
    uint16_t mismatches;
    uint16_t gaps;
} score_t;

/** @struct needleman_t
 * @brief Groups all data required for needleman execution.
 * @var seqchar The pointer to character sequences.
 * @var nseq The number of sequences loaded from file.
 * @var npair The number of working pairs received to process.
 * @var seq The sequences' positions.
 * @var pair The working pairs to process.
 */
typedef struct {
    char *seqchar;
    int8_t *table;
    uint16_t nseq = 0;
    uint32_t npair = 0;
    position_t *seq;
    workpair_t *pair;
} needleman_t;

/** @class pairwise_t
 * @brief Stores data and structures needed for executing pairwise algorithm.
 * @var seqchar The pointer to character sequences.
 * @var nseq The number of sequences loaded from file.
 * @var npair The number of working pairs received to process.
 * @var seq The sequences' positions.
 * @var pair The working pairs to process.
 * @var score The score of each working pair processed.
 */
class pairwise_t
{
public:
    char *seqchar;
    uint16_t nseq;
    uint32_t npair;
    position_t *seq;
    workpair_t *pair;
    score_t *score;

public:
    pairwise_t();
    ~pairwise_t();

    void load(const fasta_t *);
    void pairwise();

private:
    void scatter();
    bool select(bool[], std::vector<uint32_t>&) const;
    void blosum(needleman_t&);
    void run(needleman_t&);

    void alloc(needleman_t&, std::vector<uint32_t>&) const;
    void allocseq(needleman_t&, std::vector<uint16_t>&) const;
    void free(needleman_t&) const;
};

/** @fn int divceil(int, int)
 * @brief Calculates the division between two numbers and rounds it up.
 * @param a The number to be divided.
 * @param b The number to divide by.
 * @return The resulting number.
 */
inline int divceil(int a, int b)
{
    return (a / b) + !!(a % b);
}

/** @fn int align(int, int)
 * @brief Calculates the alignment for a given size.
 * @param size The size to be aligned.
 * @param align The alignment to use for given size.
 * @return The new aligned size.
 */
inline int align(int size, int align = 4)
{
    return divceil(size, align) * align;
}

namespace pairwise
{
#ifdef __CUDACC__
    extern __global__ void needleman(needleman_t, score_t *);
#endif
}

#endif