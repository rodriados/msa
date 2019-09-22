/** 
 * Multiple Sequence Alignment utilities header file.
 * @author Rodrigo Siqueira <rodriados@gmail.com>
 * @copyright 2018-2019 Rodrigo Siqueira
 */
#pragma once

#ifndef UTILS_HPP_INCLUDED
#define UTILS_HPP_INCLUDED

#include <cstddef>
#include <utility>

/*
 * Creation of conditional macros that allow CUDA declarations to be used
 * seamlessly throughout the code without any problems.
 */
#if !defined(__host__) && !defined(__device__)
  #define __host__
  #define __device__
#endif

#if defined(__CUDACC__) && !defined(msa_compile_cuda)
  #define msa_compile_cuda 1
#endif

/**#@+
 * Wraps a function pointer into a functor.
 * @tparam F The full function signature type.
 * @tparam R The function return type.
 * @tparam P The function parameter types.
 * @since 0.1.1
 */
template <typename F>
class Functor;

template <typename R, typename ...P>
class Functor<R(P...)>
{
    protected:
        using Funcptr = R (*)(P...);   /// The function pointer type.
        Funcptr func = nullptr;        /// The function represented by the functor.

    public:
        __host__ __device__ inline constexpr Functor() noexcept = default;
        __host__ __device__ inline constexpr Functor(const Functor&) noexcept = default;
        __host__ __device__ inline constexpr Functor(Functor&&) noexcept = default;

        /**
         * Constructs a new functor.
         * @param funcptr The function pointer to be carried by functor.
         */
        __host__ __device__ inline constexpr Functor(Funcptr funcptr) noexcept
        :   func {funcptr}
        {}

        __host__ __device__ inline Functor& operator=(const Functor&) noexcept = default;
        __host__ __device__ inline Functor& operator=(Functor&&) noexcept = default;

        /**
         * The functor call operator.
         * @tparam T The given parameter types.
         * @param param The given functor parameters.
         * @return The functor return value.
         */
        template <typename ...T>
        __host__ __device__ inline constexpr R operator()(T&&... param) const
        {
            return func(std::forward<decltype(param)>(param)...);
        }

        /**
         * Checks whether the functor is empty or not.
         * @return Is the functor empty?
         */
        __host__ __device__ inline constexpr bool isEmpty() const noexcept
        {
            return func == nullptr;
        }
};
/**#@-*/

/**
 * A general memory storage container.
 * @tparam S The number of bytes in storage.
 * @tparam A The byte alignment the storage should use.
 * @since 0.1.1
 */
template <size_t S, size_t A = S>
struct alignas(A) Storage
{
    alignas(A) char storage[S];     /// The storage container.
};

/**#@+
 * Represents and generates a type index sequence.
 * @tparam I The index sequence.
 * @tparam L The length of sequence to generate.
 * @since 0.1.1
 */
template <size_t ...I>
struct Indexer
{
    /**
     * The indexer sequence type.
     * @since 0.1.1
     */
    using type = Indexer;
};

template <>
struct Indexer<0>
{
    /**
     * The indexer base generator type.
     * @since 0.1.1
     */
    using type = Indexer<>;
};

template <>
struct Indexer<1>
{
    /**
     * The indexer base generator type.
     * @since 0.1.1
     */
    using type = Indexer<0>;
};

template <size_t L>
struct Indexer<L>
{
    /**
     * Concatenates two type index sequences into one.
     * @tparam I The first index sequence to merge.
     * @tparam J The second index sequence to merge.
     * @return The concatenated index sequence.
     */
    template <size_t ...I, size_t ...J>
    __host__ __device__ static constexpr auto concat(Indexer<I...>, Indexer<J...>) noexcept
    -> typename Indexer<I..., sizeof...(I) + J...>::type;

    /**
     * The indexer generator type.
     * @since 0.1.1
     */
    using type = decltype(concat(
            typename Indexer<L / 2>::type {}
        ,   typename Indexer<L - L / 2> ::type {}
        ));
};
/**#@-*/

/**
 * The type index sequence generator of given size.
 * @tparam L The length of index sequence to generate.
 * @since 0.1.1
 */
template <size_t L>
using IndexerG = typename Indexer<L>::type;

#include "operator.hpp"

namespace utils
{
    using namespace op;

    /**#@+
     * Checks whether all given values are true.
     * @param head The first value to test.
     * @param tail The following values to test.
     * @return Are all values true?
     */
    __host__ __device__ inline constexpr bool all() noexcept
    {
        return true;
    }

    template <typename T, typename ...U>
    __host__ __device__ inline constexpr bool all(T&& head, U&&... tail) noexcept
    {
        return op::andl(bool(head), all(tail...));
    }
    /**#@-*/

    /**#@+
     * Checks whether at least one given value is true.
     * @param head The first value to test.
     * @param tail The following values to test.
     * @return Is at least one value true?
     */
    __host__ __device__ inline constexpr bool any() noexcept
    {
        return false;
    }

    template <typename T, typename ...U>
    __host__ __device__ inline constexpr bool any(T&& head, U&&... tail) noexcept
    {
        return op::orl(bool(head), any(tail...));
    }
    /**#@-*/

    /**
     * Checks whether none of given values is true.
     * @param value All the values to be tested.
     * @return Are all values false?
     */
    template <typename ...T>
    __host__ __device__ inline constexpr bool none(T&&... value) noexcept
    {
        return !any(value...);
    }

    /**
     * Calculates the number of possible pair combinations with given number.
     * @param count The number of objects to be combinated.
     * @return The number of possible pair combinations.
     */
    template <typename T>
    __host__ __device__ inline constexpr T combinations(T count) noexcept
    {
        static_assert(std::is_integral<T>::value, "an integral value is expected!");
        return (count * (count - 1)) >> 1;
    }
};

/**
 * Purifies the type to its base, removing all extents it might have.
 * @tparam T The type to have its base extracted.
 * @since 0.1.1
 */
template <typename T>
using Base = typename std::remove_extent<T>::type;

/**
 * Purifies an array type to its base.
 * @tparam T The type to be purified.
 * @since 0.1.1
 */
template <typename T>
using Pure = typename std::conditional<
        std::is_array<T>::value && !std::extent<T>::value
    ,   Base<T>
    ,   typename std::remove_reference<T>::type
    >::type;

/**
 * Returns the first type unchanged. This is useful to produce a repeating list
 * of the given type.
 * @tpatam T The type to return.
 * @since 0.1.1
 */
template <typename T, size_t = 0>
using Identity = T;

#endif