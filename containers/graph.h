#pragma once

#include <concepts>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <cstddef>

namespace graph {

// Forward declarations
template<typename NodeTraits>
class CNode;

template<typename EdgeTraits>
class CEdge;

// Concepts for NodeTraits and EdgeTraits
template<typename T>
concept NodeTraitsConcept = requires {
    typename T::id_type;                // Type for node identifiers
    typename T::value_type;              // Type of data stored in node (optional)
    // possibly other requirements
};

template<typename T>
concept EdgeTraitsConcept = requires {
    typename T::id_type;                 // Type for edge identifiers
    typename T::node_id_type;             // Type for node identifiers (should match NodeTraits::id_type)
    typename T::weight_type;              // Type for edge weight (optional)
    // possibly other requirements
};

// Concept for GraphTraits
template<typename T>
concept GraphTraitsConcept = requires {
    typename T::Node;                      // Must be a CNode<...> instantiation
    typename T::Edge;                      // Must be a CEdge<...> instantiation
    // Node and Edge should have appropriate traits? Could be checked later.
};

// Default node traits
struct DefaultNodeTraits {
    using id_type = std::size_t;
    using value_type = int;  // dummy
};

// Default edge traits
struct DefaultEdgeTraits {
    using id_type = std::size_t;
    using node_id_type = std::size_t;
    using weight_type = double;
};

// CNode template
template<NodeTraitsConcept NodeTraits = DefaultNodeTraits>
class CNode {
public:
    using traits_type = NodeTraits;
    using id_type = typename NodeTraits::id_type;
    using value_type = typename NodeTraits::value_type;

    // Constructors
    explicit CNode(id_type id) : id_(id) {}
    CNode(id_type id, value_type data) : id_(id), data_(std::move(data)) {}

    // Getters
    id_type id() const noexcept { return id_; }
    value_type& data() noexcept { return data_; }
    const value_type& data() const noexcept { return data_; }

    // Setters
    void set_data(const value_type& new_data) { data_ = new_data; }

    // Possibly other methods...

private:
    id_type id_;
    value_type data_;
};

// CEdge template
template<EdgeTraitsConcept EdgeTraits = DefaultEdgeTraits>
class CEdge {
public:
    using traits_type = EdgeTraits;
    using id_type = typename EdgeTraits::id_type;
    using node_id_type = typename EdgeTraits::node_id_type;
    using weight_type = typename EdgeTraits::weight_type;

    // Constructors
    CEdge(id_type id, node_id_type src, node_id_type tgt)
        : id_(id), source_(src), target_(tgt), weight_() {}
    CEdge(id_type id, node_id_type src, node_id_type tgt, weight_type w)
        : id_(id), source_(src), target_(tgt), weight_(w) {}

    // Getters
    id_type id() const noexcept { return id_; }
    node_id_type source() const noexcept { return source_; }
    node_id_type target() const noexcept { return target_; }
    weight_type weight() const noexcept { return weight_; }

    // Setters
    void set_weight(weight_type w) noexcept { weight_ = w; }

private:
    id_type id_;
    node_id_type source_;
    node_id_type target_;
    weight_type weight_;
};

// Default graph traits that use the default node and edge traits
struct DefaultGraphTraits {
    using Node = CNode<DefaultNodeTraits>;
    using Edge = CEdge<DefaultEdgeTraits>;
};

// CGraph template
template<GraphTraitsConcept GraphTraits = DefaultGraphTraits>
class CGraph {
public:
    // Exposed types
    using graph_traits = GraphTraits;
    using node_type = typename GraphTraits::Node;
    using edge_type = typename GraphTraits::Edge;
    using node_id_type = typename node_type::id_type;
    using edge_id_type = typename edge_type::id_type;

    // Container types (can be customized via allocators later)
    using node_container = std::unordered_map<node_id_type, node_type>;
    using edge_container = std::unordered_map<edge_id_type, edge_type>;

    // Iterators
    using node_iterator = typename node_container::iterator;
    using const_node_iterator = typename node_container::const_iterator;
    using edge_iterator = typename edge_container::iterator;
    using const_edge_iterator = typename edge_container::const_iterator;

    // Constructors
    CGraph() = default;
    explicit CGraph(const node_container& nodes) : nodes_(nodes) {}
    explicit CGraph(node_container&& nodes) : nodes_(std::move(nodes)) {}

    // Rule of five (defaulted)
    ~CGraph() = default;
    CGraph(const CGraph&) = default;
    CGraph(CGraph&&) = default;
    CGraph& operator=(const CGraph&) = default;
    CGraph& operator=(CGraph&&) = default;

    // Node operations
    node_type& add_node(node_id_type id, typename node_type::value_type data = {}) {
        // Check if node already exists? Could throw or return existing.
        auto [it, inserted] = nodes_.try_emplace(id, node_type(id, std::move(data)));
        // if not inserted, handle error (e.g., throw or return existing)
        return it->second;
    }

    bool remove_node(node_id_type id) noexcept {
        // Also need to remove edges incident to this node.
        // For skeleton, just remove from nodes, leaving edges dangling.
        // Better to remove edges as well.
        return nodes_.erase(id) > 0;
    }

    node_type* find_node(node_id_type id) noexcept {
        auto it = nodes_.find(id);
        return it != nodes_.end() ? &it->second : nullptr;
    }

    const node_type* find_node(node_id_type id) const noexcept {
        auto it = nodes_.find(id);
        return it != nodes_.end() ? &it->second : nullptr;
    }

    // Edge operations
    edge_type& add_edge(edge_id_type id, node_id_type src, node_id_type tgt,
                        typename edge_type::weight_type weight = {}) {
        // Check if nodes exist? Could throw if not.
        auto [it, inserted] = edges_.try_emplace(id, edge_type(id, src, tgt, weight));
        return it->second;
    }

    bool remove_edge(edge_id_type id) noexcept {
        return edges_.erase(id) > 0;
    }

    edge_type* find_edge(edge_id_type id) noexcept {
        auto it = edges_.find(id);
        return it != edges_.end() ? &it->second : nullptr;
    }

    const edge_type* find_edge(edge_id_type id) const noexcept {
        auto it = edges_.find(id);
        return it != edges_.end() ? &it->second : nullptr;
    }

    // Iterators
    node_iterator nodes_begin() noexcept { return nodes_.begin(); }
    node_iterator nodes_end() noexcept { return nodes_.end(); }
    const_node_iterator nodes_cbegin() const noexcept { return nodes_.cbegin(); }
    const_node_iterator nodes_cend() const noexcept { return nodes_.cend(); }

    edge_iterator edges_begin() noexcept { return edges_.begin(); }
    edge_iterator edges_end() noexcept { return edges_.end(); }
    const_edge_iterator edges_cbegin() const noexcept { return edges_.cbegin(); }
    const_edge_iterator edges_cend() const noexcept { return edges_.cend(); }

    // Capacity
    size_t node_count() const noexcept { return nodes_.size(); }
    size_t edge_count() const noexcept { return edges_.size(); }
    bool empty() const noexcept { return nodes_.empty(); }

    // Clear
    void clear() noexcept {
        nodes_.clear();
        edges_.clear();
    }

private:
    node_container nodes_;
    edge_container edges_;
};

} // namespace graph
