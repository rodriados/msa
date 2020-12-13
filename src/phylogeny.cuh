/**
 * Museqa: Multiple Sequence Aligner using hybrid parallel computing.
 * @file Exposes an interface for the heuristics' phylogeny module.
 * @author Rodrigo Siqueira <rodriados@gmail.com>
 * @copyright 2019-present Rodrigo Siqueira
 */
#pragma once

#include "io.hpp"
#include "pointer.hpp"
#include "database.hpp"
#include "pipeline.hpp"
#include "pairwise.cuh"

/*
 * The heuristic's phylogeny and guiding-tree building module.
 * This module is responsible for building a pseudo-phylogenic guiding-tree, in
 * which sequences are to be joined together in order to create the best alignment
 * of all sequences at once.
 */

#include "phylogeny/phylogeny.cuh"

namespace museqa
{
    namespace module
    {
        /**
         * Defines the module's pipeline manager. This object will be the one responsible
         * for checking and managing the module's execution when on a pipeline.
         * @since 0.1.1
         */
        struct phylogeny : public pipeline::module
        {
            struct conduit;                                 /// The module's conduit type.

            typedef museqa::module::pairwise previous;      /// The expected previous module.
            typedef pointer<pipeline::conduit> pipe;        /// The generic conduit type alias.

            /**
             * Returns an string identifying the module's name.
             * @return The module's name.
             */
            inline auto name() const -> const char * override
            {
                return "phylogeny";
            }

            auto run(const io::manager&, const pipe&) const -> pipe override;
            auto check(const io::manager&) const -> bool override;
        };

        /**
         * Defines the module's conduit. This conduit is composed of the sequences
         * being aligned and the phylogenetic tree to guide their alignment.
         * @since 0.1.1
         */
        struct phylogeny::conduit : public pipeline::conduit
        {
            typedef museqa::phylogeny::tree tree;

            const pointer<database> db;     /// The loaded sequences' database.
            const tree phylotree;           /// The sequences' alignment guiding tree.
            const size_t total;             /// The total number of sequences.

            inline conduit() noexcept = delete;
            inline conduit(const conduit&) = default;
            inline conduit(conduit&&) = default;

            /**
             * Instantiates a new conduit.
             * @param db The sequence database to transfer to the next module.
             * @param ptree The alignment guiding tree to transfer to the next module.
             */
            inline conduit(const pointer<database>& db, const tree& ptree) noexcept
            :   db {db}
            ,   phylotree {ptree}
            ,   total {db->count()}
            {}

            inline conduit& operator=(const conduit&) = delete;
            inline conduit& operator=(conduit&&) = delete;
        };
    }

    namespace phylogeny
    {
        /**
         * Alias for the phylogeny module's runner.
         * @since 0.1.1
         */
        using module = museqa::module::phylogeny;

        /**
         * Alias for the phylogeny module's conduit.
         * @since 0.1.1
         */
        using conduit = museqa::module::phylogeny::conduit;
    }

    /**
     * Represents the reference for an OTU. This reference may be used to address
     * both a single sequence as well as any multiple sequence alignment.
     * @since 0.1.1
     */
    using oturef = phylogeny::oturef;

    /**
     * Represents the pseudo-phylogenetic tree generated by the phylogeny step.
     * This tree's nodes are addressed by OTUs references.
     * @since 0.1.1
     */
    using phylotree = phylogeny::tree;
}
