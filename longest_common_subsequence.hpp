// Andrew Naplavkov

#ifndef STEP_LONGEST_COMMON_SUBSEQUENCE_HPP
#define STEP_LONGEST_COMMON_SUBSEQUENCE_HPP

#include <algorithm>
#include <step/detail/common.hpp>
#include <step/detail/hirschberg.hpp>

namespace step {
namespace longest_common_subsequence {
namespace detail {

struct dynamic_programming {
    /// @see https://www.geeksforgeeks.org/longest-common-subsequence/
    template <typename RandomIt1, typename RandomIt2>
    auto make_last_row(RandomIt1 first1,
                       RandomIt1 last1,
                       RandomIt2 first2,
                       RandomIt2 last2) const
    {
        auto size1 = std::distance(first1, last1);
        auto size2 = std::distance(first2, last2);
        ring_table<size_t, 2> tbl(size2 + 1);
        for (size_t l = 1; l <= size1; ++l)
            for (size_t r = 1; r <= size2; ++r)
                tbl[l][r] = first1[l - 1] == first2[r - 1]
                                ? tbl[l - 1][r - 1] + 1
                                : std::max(tbl[l - 1][r], tbl[l][r - 1]);
        return std::move(tbl[size1]);
    }

    bool operator()(size_t lhs, size_t rhs) const { return lhs > rhs; }

    template <typename RandomIt1, typename RandomIt2, typename OutputIt>
    OutputIt trivial_align(RandomIt1 first1,
                           RandomIt1 last1,
                           RandomIt2 first2,
                           RandomIt2 last2,
                           OutputIt result) const
    {
        auto it = std::find_first_of(first1, last1, first2, last2);
        if (it != last1) {
            *result = *it;
            ++result;
        }
        return result;
    }
};

}  // namespace detail

/**
 * Find the longest subsequence present in two sequences. A subsequence is a
 * sequence that appears in the same relative order, but not necessarily
 * contiguous.
 *
 * Time complexity: O(N*M), space complexity O(min(N,M)), where:
 * N = std::distance(first1, last1), M = std::distance(first2, last2).
 */
template <typename RandomIt1, typename RandomIt2, typename OutputIt>
OutputIt intersect(RandomIt1 first1,
                   RandomIt1 last1,
                   RandomIt2 first2,
                   RandomIt2 last2,
                   OutputIt result)
{
    return hirschberg::align(
        first1, last1, first2, last2, result, detail::dynamic_programming{});
}

template <typename RandomRng1, typename RandomRng2, typename OutputIt>
OutputIt intersect(const RandomRng1& rng1,
                   const RandomRng2& rng2,
                   OutputIt result)
{
    return longest_common_subsequence::intersect(std::begin(rng1),
                                                 std::end(rng1),
                                                 std::begin(rng2),
                                                 std::end(rng2),
                                                 result);
}

}  // namespace longest_common_subsequence
}  // namespace step

#endif  // STEP_LONGEST_COMMON_SUBSEQUENCE_HPP