#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__
#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <fstream>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <shared_mutex>
#include <type_traits>
#include <functional>
using namespace std;
#include "../types.h"
#include "traits.h"
#include "stack.h"

// ─── BinaryTreeNode (CRTP) ───────────────────────────────────────────────────
// m_pChild[0] = right (larger en ascending), m_pChild[1] = left (smaller)
template<typename T, typename NodeType = void>
struct BinaryTreeNode {
    using value_type = T;
    using Node = conditional_t<is_void_v<NodeType>, BinaryTreeNode, NodeType>;
    T     m_data;
    Ref   m_ref;
    Node* m_pChild[2];
    BinaryTreeNode(T data, Ref ref)
        : m_data(data), m_ref(ref), m_pChild{nullptr, nullptr} {}
    T&   getDataRef()   { return m_data; }
    Ref  getRef() const { return m_ref;  }
};

// Traits genéricos para BinaryTree
template<typename T>
struct AscendingBTTrait  : BaseTrait<BinaryTreeNode<T>, less<T>>    {};
template<typename T>
struct DescendingBTTrait : BaseTrait<BinaryTreeNode<T>, greater<T>> {};

// ─── fill helpers ────────────────────────────────────────────────────────────
// m_pChild[1] = left (menor en ascending), m_pChild[0] = right (mayor)
template<typename Node>
void fill_inorder(Node* n, Stack<Node*>& s) {
    if(!n) return;
    fill_inorder(n->m_pChild[1], s);
    s.push(n);
    fill_inorder(n->m_pChild[0], s);
}

template<typename Node>
void fill_preorder(Node* n, Stack<Node*>& s) {
    if(!n) return;
    s.push(n);
    fill_preorder(n->m_pChild[1], s);
    fill_preorder(n->m_pChild[0], s);
}

template<typename Node>
void fill_postorder(Node* n, Stack<Node*>& s) {
    if(!n) return;
    fill_postorder(n->m_pChild[1], s);
    fill_postorder(n->m_pChild[0], s);
    s.push(n);
}

// ─── TreeIteratorBase ──────────────────────────────────────────────────────────
template<typename Node>
class TreeIteratorBase {
protected:
    Stack<Node*> m_nodes;
    ptrdiff_t    m_index;
public:
    TreeIteratorBase() : m_index(0) {}
    TreeIteratorBase(Stack<Node*> nodes, ptrdiff_t idx)
        : m_nodes(move(nodes)), m_index(idx) {}

    typename Node::value_type& operator*() {
        return m_nodes[m_index]->getDataRef();
    }
    Node* node() const { return m_nodes[m_index]; }
    bool operator==(const TreeIteratorBase& o) const { return m_index == o.m_index; }
    bool operator!=(const TreeIteratorBase& o) const { return m_index != o.m_index; }
};

// ─── TreeForwardIterator ───────────────────────────────────────────────────────
template<typename Node>
class TreeForwardIterator : public TreeIteratorBase<Node> {
public:
    using TreeIteratorBase<Node>::TreeIteratorBase;
    TreeForwardIterator& operator++() { ++this->m_index; return *this; }
};

// ─── TreeReverseIterator ──────────────────────────────────────────────────────
template<typename Node>
class TreeReverseIterator : public TreeIteratorBase<Node> {
public:
    using TreeIteratorBase<Node>::TreeIteratorBase;
    TreeReverseIterator& operator++() { --this->m_index; return *this; }
};

// ─── TreeSnapshot ───────────────────────────────────────────────────────────
template<typename Node>
struct TreeSnapshot {
    TreeForwardIterator<Node>  m_begin, m_end;
    TreeReverseIterator<Node> m_rbegin, m_rend;

    TreeForwardIterator<Node>  begin()  { return m_begin;  }
    TreeForwardIterator<Node>  end()    { return m_end;    }
    TreeReverseIterator<Node> rbegin() { return m_rbegin; }
    TreeReverseIterator<Node> rend()   { return m_rend;   }

    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        for(auto it = m_begin; it != m_end; ++it)
            func(*it, forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    void rForEach(Func func, Args&&... args) {
        for(auto it = m_rbegin; it != m_rend; ++it)
            func(*it, forward<Args>(args)...);
    }
};

template<typename Node>
TreeSnapshot<Node> make_view(Stack<Node*> s) {
    size_t n = s.size();
    Stack<Node*> s2 = s;
    TreeSnapshot<Node> v;
    v.m_begin  = TreeForwardIterator<Node> (move(s),  0);
    v.m_end    = TreeForwardIterator<Node> ({},        (ptrdiff_t)n);
    v.m_rbegin = TreeReverseIterator<Node>(move(s2), (ptrdiff_t)n - 1);
    v.m_rend   = TreeReverseIterator<Node>({},        -1);
    return v;
}

// ─── BinaryTree ──────────────────────────────────────────────────────────────
template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

protected:
    Node*  m_pRoot = nullptr;
    Comp   m_comp;
    mutable shared_mutex m_mtx;

    virtual void internal_insert(Node*& pNode, const value_type& data, Ref ref) {
        if(!pNode) { pNode = new Node(data, ref); return; }
        auto branch = !m_comp(pNode->m_data, data);
        internal_insert(pNode->m_pChild[branch], data, ref);
    }

    virtual void internal_copy(Node*& dst, Node* src) {
        if(!src) { dst = nullptr; return; }
        dst = new Node(src->m_data, src->m_ref);
        internal_copy(dst->m_pChild[0], src->m_pChild[0]);
        internal_copy(dst->m_pChild[1], src->m_pChild[1]);
    }

    virtual void internal_clear(Node* node) {
        if(!node) return;
        internal_clear(node->m_pChild[0]);
        internal_clear(node->m_pChild[1]);
        delete node;
    }

    // Sin lock: lo invoca el caller que ya tomo el lock. Devuelve el nodo o nullptr.
    Node* find_node(const value_type& key) const {
        Node* node = m_pRoot;
        while(node) {
            if(!m_comp(node->m_data, key) && !m_comp(key, node->m_data)) return node;
            node = node->m_pChild[!m_comp(node->m_data, key)];
        }
        return nullptr;
    }

public:
    BinaryTree() {}

    BinaryTree(const BinaryTree& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        internal_copy(m_pRoot, other.m_pRoot);
    }

    BinaryTree(BinaryTree&& other) noexcept {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = exchange(other.m_pRoot, nullptr);
    }

    virtual ~BinaryTree() {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_clear(m_pRoot);
        m_pRoot = nullptr;
    }

    void insert(const value_type& data, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, data, ref);
    }

    // Mejora libre #1: search devuelve tuple<value_type, Ref>
    tuple<value_type, Ref> search(const value_type& data) {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* node = m_pRoot;
        while(node) {
            if(!m_comp(node->m_data, data) && !m_comp(data, node->m_data))
                return {node->m_data, node->m_ref};
            node = node->m_pChild[!m_comp(node->m_data, data)];
        }
        throw runtime_error("BinaryTree::search: not found");
    }

    // Traversals con shared_lock — devuelven snapshot del árbol
    TreeSnapshot<Node> inorder() {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s; fill_inorder(m_pRoot, s);
        return make_view(move(s));
    }

    TreeSnapshot<Node> preorder() {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s; fill_preorder(m_pRoot, s);
        return make_view(move(s));
    }

    TreeSnapshot<Node> postorder() {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s; fill_postorder(m_pRoot, s);
        return make_view(move(s));
    }

    // begin/end delegan a inorder -> range-based for usa inorder por defecto
    TreeForwardIterator<Node>  begin()  { return inorder().m_begin;  }
    TreeForwardIterator<Node>  end()    { return inorder().m_end;    }
    TreeReverseIterator<Node> rbegin() { return inorder().m_rbegin; }
    TreeReverseIterator<Node> rend()   { return inorder().m_rend;   }

    template<typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s; fill_inorder(m_pRoot, s);
        for(size_t i = 0; i < s.size(); ++i)
            func(s[i]->getDataRef(), forward<Args>(args)...);
    }

    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s;
        fill_inorder(m_pRoot, s);
        ostringstream oss;
        oss << "[";
        for(size_t i = 0; i < s.size(); ++i) {
            if(i) oss << ",";
            oss << "(" << s[i]->m_data << "," << s[i]->m_ref << ")";
        }
        oss << "]";
        return oss.str();
    }

    // Mejora libre #2: printTree por niveles (BFS)
    void printTree(ostream& os) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if(!m_pRoot) { os << "(empty)\n"; return; }
        Vector<VectorTrait<Node*>> queue;
        queue.push_back(m_pRoot, 0);
        size_t levelStart = 0;
        while(levelStart < queue.size()) {
            size_t levelEnd = queue.size();
            for(size_t i = levelStart; i < levelEnd; ++i) {
                Node* n = queue.get(i);
                os << n->m_data << " ";
                if(n->m_pChild[1]) queue.push_back(n->m_pChild[1], 0);
                if(n->m_pChild[0]) queue.push_back(n->m_pChild[0], 0);
            }
            os << "\n";
            levelStart = levelEnd;
        }
    }

    friend ostream& operator<<(ostream& os, const BinaryTree& t) {
        shared_lock<shared_mutex> lock(t.m_mtx);
        Stack<Node*> s; fill_inorder(t.m_pRoot, s);
        os << "[";
        for(size_t i = 0; i < s.size(); ++i) {
            if(i) os << ",";
            os << "(" << s[i]->m_data << "," << s[i]->m_ref << ")";
        }
        os << "]";
        return os;
    }

    friend istream& operator>>(istream& is, BinaryTree& t) {
        char ch;
        if(!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type val; Ref ref; char comma, close;
        while(is >> ch && ch != ']')
            if(ch == '(')
                if(is >> val >> comma >> ref >> close)
                    if(comma == ',' && close == ')')
                        t.insert(val, ref);
        return is;
    }
};

#endif // __BINARYTREE_H__
