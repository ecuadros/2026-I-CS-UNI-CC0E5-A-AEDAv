#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <shared_mutex>
#include <type_traits>
#include "stack.h"
#include "vector.h"
#include "../types.h"
#include "traits.h"
using namespace std;

template <typename T, typename Derived>
struct NodeBase {
    using value_type = T;
    
    T m_data;
    Ref m_ref;
    Derived* m_child[2];

    NodeBase(T data, Ref ref) : m_data(data), m_ref(ref), m_child{nullptr, nullptr} {}

    friend std::ostream& operator<<(std::ostream& os, const NodeBase& node) {
        os << "(" << node.m_data << "," << node.m_ref << ")";
        return os;
    }
};

// BTNode
template<typename T>
struct BTNode : public NodeBase<T, BTNode<T>> {
    using NodeBase<T, BTNode<T>>::NodeBase;
};

// BTIteratorBase
template<typename Node, typename value_type>
class BTIteratorBase {
protected:
    const Stack<Node*>* m_nodes_ptr;
    size_t m_pos;

public:
    BTIteratorBase() : m_nodes_ptr(nullptr), m_pos(0) {}
    BTIteratorBase(const Stack<Node*>* s_ptr, size_t pos) : m_nodes_ptr(s_ptr), m_pos(pos) {}

    value_type& operator*() const { return (*m_nodes_ptr)[m_pos]->m_data; }
    Node* getNode() const { return  (*m_nodes_ptr)[m_pos]; }

    bool operator==(const BTIteratorBase& o) const { return m_pos == o.m_pos && m_nodes_ptr == o.m_nodes_ptr; }
    bool operator!=(const BTIteratorBase& o) const { return !(*this == o); }
};

// Forward iterator
template<typename Node, typename value_type>
class BTForwardIterator : public BTIteratorBase<Node, value_type> {
public:
    using BTIteratorBase<Node, value_type>::BTIteratorBase;
    BTForwardIterator& operator++() { ++this->m_pos; return *this; }
};

// Backward iterator
template<typename Node, typename value_type>
class BTBackwardIterator : public BTIteratorBase<Node, value_type> {
public:
    using BTIteratorBase<Node, value_type>::BTIteratorBase;
    BTBackwardIterator& operator++() { --this->m_pos; return *this; }
};

template<typename Node, typename value_type>
class TraversalView {
private:
    Stack<Node*> m_data;

public:
    using ForwardIt = BTForwardIterator<Node, value_type>;
    using BackwardIt = BTBackwardIterator<Node, value_type>;
    TraversalView(Stack<Node*>&& s) : m_data(std::move(s)) {}

    ForwardIt begin() const { return ForwardIt(&m_data, 0); }
    ForwardIt end() const   { return ForwardIt(&m_data, m_data.size()); }
    BackwardIt rbegin() const { return BackwardIt(&m_data, m_data.size() - 1); }
    BackwardIt rend() const   { return BackwardIt(&m_data, (size_t)-1); }

    struct ReverseView {
        Stack<Node*> m_data;
        ReverseView(Stack<Node*>&& s) : m_data(std::move(s)) {}
        auto begin() const { return BackwardIt(&m_data, m_data.size() - 1); }
        auto end() const   { return BackwardIt(&m_data, (size_t)-1); }
    };

    ReverseView reversed() && {
        return ReverseView(std::move(this->m_data));
    }

    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) const {
        for (auto it = begin(); it != end(); ++it)
            func(*it, std::forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    void rForEach(Func func, Args&&... args) const {
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it, std::forward<Args>(args)...);
    }
};

// BinaryTree
template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node = typename Trait::Node;
    using Comp = typename Trait::Comp;

    using ForwardIt = BTForwardIterator <Node, value_type>;
    using BackwardIt = BTBackwardIterator<Node, value_type>;

    using InorderView = TraversalView<Node, value_type>;
    using PreorderView = TraversalView<Node, value_type>;
    using PostorderView = TraversalView<Node, value_type>;

protected:
    Node *m_root;
    Comp m_cmp;
    mutable shared_mutex m_lock;

    // internal_insert
    virtual void internal_insert(Node* &node, const value_type &data, Ref ref) {
        if (!node) { node = new Node(data, ref); return; }
        auto branch = !m_cmp(data, node->m_data);
        internal_insert(node->m_child[branch], data, ref);
    }

    // Destructor seguro
    virtual void internal_clear(Node* node) {
        if (!node) return;
        internal_clear(node->m_child[0]);
        internal_clear(node->m_child[1]);
        delete node;
    }

    // Copy constructor
    virtual Node* internal_copy(Node* node) {
        if (!node) return nullptr;
        Node* n = new Node(node->m_data, node->m_ref);
        n->m_child[0] = internal_copy(node->m_child[0]);
        n->m_child[1] = internal_copy(node->m_child[1]);
        return n;
    }

    // internal_search
    virtual Node* internal_search(Node* node, const value_type& data) const {
        if (!node) return nullptr;
        if (!m_cmp(data, node->m_data) && !m_cmp(node->m_data, data))
            return node;
        auto branch = !m_cmp(data, node->m_data);
        return internal_search(node->m_child[branch], data);
    }

    virtual size_t internal_size(Node* node) const {
        if (!node) return 0;
        return 1 + internal_size(node->m_child[0]) + internal_size(node->m_child[1]);
    }

    // mejora 1 : check_balance
    pair<size_t, bool> check_balance(Node* node) const {
        if (!node) return {0, true}; 
        auto [h_left, bal_left]   = check_balance(node->m_child[0]);
        auto [h_right, bal_right] = check_balance(node->m_child[1]);
        size_t diff = (h_left > h_right) ? h_left - h_right : h_right - h_left;
        bool is_balanced = bal_left && bal_right && (diff <= 1);
        
        return {1 + max(h_left, h_right), is_balanced};
    }

    // mejora 2 : dfs_search
    void dfs_search(Node* node, std::function<bool(const value_type&)> pred,
                    Vector<tuple<value_type, Ref>>& results) const {
        if (!node) return;
        
        // Visita en preorder
        if (pred(node->m_data)) {
            results.push_back({node->m_data, node->m_ref}, 0);
        }
        
        dfs_search(node->m_child[0], pred, results);
        dfs_search(node->m_child[1], pred, results);
    }

    // Forward iterator inorder
    void fill_inorder(Node* node, Stack<Node*>& s) const {
        if (!node) return;
        fill_inorder(node->m_child[0], s);
        s.push(node);
        fill_inorder(node->m_child[1], s);
    }

    // Forward iterator preorder
    void fill_preorder(Node* node, Stack<Node*>& s) const {
        if (!node) return;
        s.push(node);
        fill_preorder(node->m_child[0], s);
        fill_preorder(node->m_child[1], s);
    }

    // Forward iterator postorder
    void fill_postorder(Node* node, Stack<Node*>& s) const {
        if (!node) return;
        fill_postorder(node->m_child[0], s);
        fill_postorder(node->m_child[1], s);
        s.push(node);
    }

    // ToString
    string traversalToString(Stack<Node*>& s) const {
        ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < s.size(); ++i) {
            if (i > 0) oss << ",";
            oss << *(s[i]);
        }
        oss << "]";
        return oss.str();
    }

public:
    BinaryTree() : m_root(nullptr) {}

    // Copy constructor
    BinaryTree(const BinaryTree& other) : m_root(nullptr) {
        shared_lock<shared_mutex> lock(other.m_lock);
        m_root = internal_copy(other.m_root);
    }

    // Move constructor
    BinaryTree(BinaryTree&& other) : m_root(nullptr) {
        unique_lock<shared_mutex> lock(other.m_lock);
        m_root = exchange(other.m_root, nullptr);
    }

    BinaryTree& operator=(const BinaryTree& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(this->m_lock);
            shared_lock<shared_mutex> olock(other.m_lock);
            internal_clear(this->m_root);
            this->m_root = internal_copy(other.m_root);
        }
        return *this;
    }

    BinaryTree& operator=(BinaryTree&& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(this->m_lock);
            unique_lock<shared_mutex> olock(other.m_lock);
            internal_clear(this->m_root);
            this->m_root = exchange(other.m_root, nullptr);
        }
        return *this;
    }

    // Destructor seguro
    virtual ~BinaryTree() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(m_lock);
        internal_clear(m_root);
        m_root = nullptr;
    }

    // insert
    void insert(const value_type& data, Ref ref) {
        unique_lock<shared_mutex> lock(m_lock);
        internal_insert(m_root, data, ref);
    }

    // search
    tuple<value_type, Ref> search(const value_type& data) const {
        shared_lock<shared_mutex> lock(m_lock);
        Node* found = internal_search(m_root, data);
        if (!found) throw runtime_error("elemento no encontrado");
        return {found->m_data, found->m_ref};
    }

    value_type* find_exact(const value_type& data) const {
        shared_lock<shared_mutex> lock(m_lock);
        Node* found = internal_search(m_root, data);
        if (found) {
            return &(found->m_data); // Devolvemos la dirección de memoria del dato
        }
        return nullptr;
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_lock);
        return internal_size(m_root);
    }

    // mejora 1: isBalanced
    bool isBalanced() const {
        shared_lock<shared_mutex> lock(m_lock);
        auto [height, balanced] = check_balance(m_root);
        return balanced;
    }

    // mejora 2: searchAll
    Vector<tuple<value_type, Ref>> searchAll(std::function<bool(const value_type&)> pred) const {
        shared_lock<shared_mutex> lock(m_lock);
        Vector<tuple<value_type, Ref>> results(64);
        dfs_search(m_root, pred, results);
        return results;
    }

    // Forward/Backward iterator inorder
    InorderView inorder() const {
        shared_lock<shared_mutex> lock(m_lock);
        Stack<Node*> stk;
        fill_inorder(m_root, stk);
        return InorderView(std::move(stk));
    }
    
    // Forward/Backward iterator preorder
    PreorderView preorder() const {
        shared_lock<shared_mutex> lock(m_lock);
        Stack<Node*> stk;
        fill_preorder(m_root, stk);
        return PreorderView(std::move(stk));
    }

    // Forward/Backward iterator postorder
    PostorderView postorder() const {
        shared_lock<shared_mutex> lock(m_lock);
        Stack<Node*> stk;
        fill_postorder(m_root, stk);
        return PostorderView(std::move(stk));
    }

    // ToString
    string toString() const {
        shared_lock<shared_mutex> lock(m_lock);
        string result;
        {
            Stack<Node*> stk;
            fill_inorder(m_root, stk);
            result += "Inorder:" + traversalToString(stk) + "\n";
        }

        {
            Stack<Node*> stk;
            fill_preorder(m_root, stk);
            result += "Preorder:" + traversalToString(stk) + "\n";
        }
        
        {
            Stack<Node*> stk;
            fill_postorder(m_root, stk);
            result += "Postorder:" + traversalToString(stk);
        }
        return result;
    }

    // operator<<
    friend ostream& operator<<(ostream& os, const BinaryTree& tree) {
        os << "[";
        bool first = true;
        auto view = tree.inorder(); 
        for (auto it = view.begin(); it != view.end(); ++it) {
            if (!first) os << ",";
            os << *(it.getNode()); 
            first = false;
        }
        os << "]";
        return os;
    }

    // operator>>
    friend istream& operator>>(istream& is, BinaryTree& tree) {
        char c;
        if (!(is >> c) || c != '[') { is.clear(ios_base::failbit); return is; }
        value_type val; Ref ref; char comma, paren;
        while (is >> c && c != ']')
            if (c == '(')
                if (is >> val >> comma >> ref >> paren)
                    if (comma == ',' && paren == ')')
                        tree.insert(val, ref);
        return is;
    }
};

#endif // __BINARYTREE_H__
