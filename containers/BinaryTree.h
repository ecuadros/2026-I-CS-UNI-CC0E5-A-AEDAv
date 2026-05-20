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

// BTNode
template<typename T, typename DerivedNode = void>
struct BTNode {
    using value_type = T;
    using Node = std::conditional_t<std::is_void_v<DerivedNode>, BTNode, DerivedNode>;
    T m_data;
    Ref m_ref;
    Node *m_child[2];
    BTNode(T data, Ref ref) : m_data(data), m_ref(ref), m_child{nullptr, nullptr} {}
};

// BTIteratorBase
template<typename Node, typename value_type>
class BTIteratorBase {
protected:
    Stack<Node*> m_nodes;
    size_t m_pos;

public:
    BTIteratorBase() : m_pos(0) {}
    BTIteratorBase(Stack<Node*> s, size_t pos) : m_nodes(s), m_pos(pos) {}

    value_type& operator*() const { return m_nodes[m_pos]->m_data; }
    Node* getNode() const { return m_nodes[m_pos]; }

    bool operator==(const BTIteratorBase& o) const { return m_pos == o.m_pos; }
    bool operator!=(const BTIteratorBase& o) const { return m_pos != o.m_pos; }
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

template<typename ForwardIt, typename BackwardIt>
class TraversalView {
    ForwardIt m_begin;
    ForwardIt m_end;
    BackwardIt m_rbegin;
    BackwardIt m_rend;

public:
    TraversalView(ForwardIt b, ForwardIt e, BackwardIt rb, BackwardIt re)
        : m_begin(b), m_end(e), m_rbegin(rb), m_rend(re) {}

    ForwardIt begin() const { return m_begin;  }
    ForwardIt end() const { return m_end;    }
    BackwardIt rbegin() const { return m_rbegin; }
    BackwardIt rend() const { return m_rend;   }

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

    using InorderView = TraversalView<ForwardIt, BackwardIt>;
    using PreorderView = TraversalView<ForwardIt, BackwardIt>;
    using PostorderView = TraversalView<ForwardIt, BackwardIt>;

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

    TraversalView<ForwardIt, BackwardIt> make_view(Stack<Node*> s) const {
        size_t last = s.size() - 1;
        ForwardIt  b (s, 0), e (s, s.size());
        BackwardIt rb(s, last), re(s, (size_t)-1);
        return { b, e, rb, re };
    }

    // ToString
    string traversalToString(Stack<Node*>& s) const {
        ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < s.size(); ++i) {
            if (i) oss << ",";
            oss << "(" << s[i]->m_data << "," << s[i]->m_ref << ")";
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
            clear();
            shared_lock<shared_mutex> lock(other.m_lock);
            m_root = internal_copy(other.m_root);
        }
        return *this;
    }

    BinaryTree& operator=(BinaryTree&& other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_lock);
            m_root = exchange(other.m_root, nullptr);
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
        return make_view(stk);
    }
    
    // Forward/Backward iterator preorder
    PreorderView preorder() const {
        shared_lock<shared_mutex> lock(m_lock);
        Stack<Node*> stk;
        fill_preorder(m_root, stk);
        return make_view(stk);
    }

    // Forward/Backward iterator postorder
    PostorderView postorder() const {
        shared_lock<shared_mutex> lock(m_lock);
        Stack<Node*> stk;
        fill_postorder(m_root, stk);
        return make_view(stk);
    }

    ForwardIt begin() const { return inorder().begin(); }
    ForwardIt end()   const { return inorder().end();   }

    // ToString
    string toString() const {
        shared_lock<shared_mutex> lock(m_lock);
        Stack<Node*> stk;
        string result;
        fill_inorder(m_root, stk);
        result += "Inorder:"   + traversalToString(stk) + "\n";
        while (!stk.empty()) stk.pop();
        fill_preorder(m_root, stk);
        result += "Preorder:"  + traversalToString(stk) + "\n";
        while (!stk.empty()) stk.pop();
        fill_postorder(m_root, stk);
        result += "Postorder:" + traversalToString(stk);
        return result;
    }

    // operator<<
    friend ostream& operator<<(ostream& os, const BinaryTree& tree) {
        shared_lock<shared_mutex> lock(tree.m_lock);
        Stack<Node*> stk;
        tree.fill_inorder(tree.m_root, stk);
        os << tree.traversalToString(stk);
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
