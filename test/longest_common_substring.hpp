// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_COMMON_SUBSTRING_HPP
#define STEP_TEST_LONGEST_COMMON_SUBSTRING_HPP

#include <map>
#include <step/longest_common_substring.hpp>
#include <step/test/utility.hpp>
#include <string_view>

TEST_CASE("longest_common_substring_hello_world")
{
    auto range = step::longest_common_substring::find_with_suffix_tree(
        "the longest string that is #", "a substring of two strings $");
    CHECK(" string" == std::string(range.first, range.second));
}

TEST_CASE("longest_common_substring_find")
{
    struct {
        std::string_view lhs;
        std::string_view rhs;
        std::string_view expect;
    } tests[] = {
        {"xabxac#", "abcabxabcd$", "abxa"},
        {"xabxaabxa#", "babxba$", "abx"},
        {"GeeksforGeeks#", "GeeksQuiz$", "Geeks"},
        {"OldSite:GeeksforGeeks.org#", "NewSite:GeeksQuiz.com$", "Site:Geeks"},
        {"abcde#", "fghie$", "e"},
        {"pqrst#", "uvwxyz$", ""},
    };
    for (auto& [lhs, rhs, expect] : tests) {
        auto arr_rng =
            step::longest_common_substring::find_with_suffix_array(lhs, rhs);
        CHECK(expect == std::string{arr_rng.first, arr_rng.second});

        auto tree_rng =
            step::longest_common_substring::find_with_suffix_tree<std::map>(
                lhs, rhs);
        CHECK(expect == std::string{tree_rng.first, tree_rng.second});
    }
}

TEST_CASE("longest_common_substring_case_insensitive")
{
    using namespace std::literals;

    const auto str1 = "geeksforGeeks#"sv;
    const auto str2 = "GEEKSQUIZ$"sv;
    const auto expect = "geeks"s;

    auto arr_rng = step::longest_common_substring::find_with_suffix_array<
        case_insensitive_less>(str1, str2);
    CHECK(expect == std::string(arr_rng.first, arr_rng.second));

    auto tree_rng = step::longest_common_substring::find_with_suffix_tree<
        case_insensitive_unordered_map>(str1, str2);
    CHECK(expect == std::string(tree_rng.first, tree_rng.second));
}

#endif  // STEP_TEST_LONGEST_COMMON_SUBSTRING_HPP
