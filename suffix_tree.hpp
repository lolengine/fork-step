// Andrew Naplavkov

#ifndef STEP_SUFFIX_TREE_HPP
#define STEP_SUFFIX_TREE_HPP

#include <functional>
#include <limits>
#include <stack>
#include <step/detail/utility.hpp>
#include <unordered_map>

namespace step {

/**
 * Ukkonen's online algorithm for constructing suffix tree.
 * Time complexity O(N*log(N)), space complexity O(N),
 * where N is length of text.
 * @param T - type of the characters;
 * @param Size - to specify the maximum number of characters;
 * @param Map - to associate characters with edges, its key_type shall be T.
 * @see https://en.wikipedia.org/wiki/Suffix_tree
 */
template <class T = char,
          class Size = size_t,
          template <class...> class Map = std::unordered_map>
class suffix_tree {
public:
    using value_type = T;
    using size_type = Size;
    using substring = std::pair<Size, Size>;  // position range

    Size size() const { return str_.size(); }
    auto data() const { return str_.data(); }

    static Size size(const substring& str) { return str.second - str.first; }
    auto begin(const substring& str) const { return data() + str.first; }
    auto end(const substring& str) const { return data() + str.second; }
    bool suffix(const substring& str) const { return str.second == size(); }

    void clear() noexcept
    {
        str_.clear();
        nodes_.clear();
        pos_ = 0;
        link_ = 0;
    }

    void reserve(Size len)
    {
        str_.reserve(len);
        nodes_.reserve(len);
    }

    /// Basic exception guarantee.
    void push_back(T val) try {
        str_.push_back(val);
        if (nodes_.empty())
            nodes_.emplace_back();  // root
        auto connect = [this, last = nodes_.size()](Size link) mutable {
            if (last < nodes_.size() && last != link)
                nodes_[last++].link = link;
        };
        while (reminder(pos_)) {
            if (auto& edge = nodes_[link_].edges[str_[pos_]]; edge) {
                if (walk_down(edge))
                    continue;
                if (!split(edge))
                    return connect(link_);
                connect(nodes_.size() - 1);
            }
            else {
                edge = inverse(pos_);
                connect(link_);
            }
            if (link_)
                link_ = nodes_[link_].link;
            else
                ++pos_;
        }
    }
    catch (...) {
        clear();
        throw;
    }

    /**
     * Find position of the first occurrence of the pattern as substring
     * in O(M) time, where M is length of the pattern.
     */
    template <class InputIt>
    Size find(InputIt first, InputIt last) const
    {
        auto way = find_path(first, last);
        return substr(way.link).second - way.len;
    }

    template <class InputRng>
    Size find(const InputRng& rng) const
    {
        return find(std::begin(rng), std::end(rng));
    }

    /**
     * Find all occurrences of the pattern for explicit suffix tree
     * (padded with a terminal symbol not seen in the string).
     */
    template <class InputIt>
    auto find_all(InputIt first, InputIt last) const
    {
        std::vector<Size> result;
        dfs(find_path(first, last),
            [&](const auto& str, const auto&, auto len) {
                if (suffix(str))
                    result.push_back(str.second - len);
            },
            [](auto&&...) {});
        return result;
    }

    template <class InputRng>
    auto find_all(const InputRng& rng) const
    {
        return find_all(std::begin(rng), std::end(rng));
    }

    /**
     * Depth-first traversal of the nodes.
     * @param pre and @param post visitors have signature:
     * @param str - substring of the node;
     * @param parent_str - substring of the parent node;
     * @param len - number of characters on the path from the root to the node.
     */
    template <class PreVisitor, class PostVisitor>
    void visit(PreVisitor pre, PostVisitor post) const
    {
        dfs(nodes_.empty() ? path{npos} : path{}, pre, post);
    }

private:
    inline static const auto npos = std::numeric_limits<Size>::max();
    inline static const auto eq = key_equal_or_equivalence<Map<T, Size>>{};

    struct node {
        Map<T, Size> edges;
        substring str;
        Size link;
    };

    std::vector<T> str_;
    std::vector<node> nodes_;  // internal nodes
    Size pos_ = 0;             // active edge character
    Size link_ = 0;            // active node

    static Size inverse(Size n) { return n == npos ? npos : npos - n - 1; }
    Size reminder(Size pos) const { return size() - pos; }
    bool leaf(Size link) const { return link >= nodes_.size(); }

    auto substr(Size link) const
    {
        return leaf(link) ? substring{inverse(link), size()} : nodes_[link].str;
    }

    bool walk_down(Size link)
    {
        auto len = size(substr(link));
        if (reminder(pos_) <= len)
            return false;
        pos_ += len;
        link_ = link;
        return true;
    }

    bool split(Size& edge)
    {
        auto str = substr(edge);
        auto head = substring{str.first, str.first + reminder(pos_) - 1};
        auto tail = substring{head.second, str.second};
        if (eq(str_[tail.first], str_.back()))
            return false;
        Size link = edge;
        edge = nodes_.size();
        nodes_.push_back({{{str_.back(), inverse(size() - 1)}}, head});
        if (leaf(link))
            nodes_.back().edges[str_[tail.first]] = inverse(tail.first);
        else {
            nodes_.back().edges[str_[tail.first]] = link;
            nodes_[link].str = tail;
        }
        return true;
    }

    struct path {
        Size link;
        Size parent_link;
        Size len;
        bool visited;
    };

    template <class InputIt>
    path find_path(InputIt first, InputIt last) const
    {
        if (nodes_.empty())
            return {npos};
        path result{};  // root
        while (true) {
            auto str = substr(result.link);
            result.len += size(str);
            auto diff = std::mismatch(first, last, begin(str), end(str), eq);
            if (diff.first == last)
                return result;
            if (diff.second != end(str) || leaf(result.link))
                return {npos};
            auto& edges = nodes_[result.link].edges;
            auto it = edges.find(*diff.first);
            if (it == edges.end())
                return {npos};
            first = diff.first;
            result.parent_link = result.link;
            result.link = it->second;
        }
    }

    template <class PreVisitor, class PostVisitor>
    void dfs(const path& root, PreVisitor pre, PostVisitor post) const
    {
        std::stack<path> stack;
        if (root.link != npos)
            stack.push(root);
        while (!stack.empty()) {
            if (auto& way = stack.top(); way.visited) {
                post(substr(way.link), substr(way.parent_link), way.len);
                stack.pop();
            }
            else {
                pre(substr(way.link), substr(way.parent_link), way.len);
                way.visited = true;
                if (!leaf(way.link)) {
                    std::stack<path> reversed;
                    for (auto& [key, edge] : nodes_[way.link].edges)
                        reversed.push({edge,
                                       way.link,
                                       Size(way.len + size(substr(edge)))});
                    for (; !reversed.empty(); reversed.pop())
                        stack.push(reversed.top());
                }
            }
        }
    }
};

}  // namespace step

#endif  // STEP_SUFFIX_TREE_HPP
