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
#include "general_iterator.h"

// ─── BinaryTreeNode (CRTP) ───────────────────────────────────────────────────
// m_pChild[0] = right (larger en ascending), m_pChild[1] = left (smaller)
template<typename T, typename NodeType = void>
struct BinaryTreeNode {
    using value_type = T;
    using Node = conditional_t<is_void_v<NodeType>, BinaryTreeNode, NodeType>;
    T     m_data;
    Ref   m_ref;
    Node* m_pChild[2];
    Node* m_pParent;
    BinaryTreeNode(T data, Ref ref, Node* parent = nullptr)
        : m_data(data), m_ref(ref), m_pChild{nullptr, nullptr}, m_pParent(parent) {}
    T&   getDataRef()   { return m_data; }
    Ref  getRef() const { return m_ref;  }
    string toString() const {
        ostringstream oss;
        oss << "(" << m_data << "," << m_ref << ")";
        return oss.str();
    }
};

// Traits genéricos para BinaryTree
template<typename T>
struct AscendingBTTrait  : BaseTrait<BinaryTreeNode<T>, less<T>>    {};
template<typename T>
struct DescendingBTTrait : BaseTrait<BinaryTreeNode<T>, greater<T>> {};

// Iteradores. L=1 forward, L=0 reverse; !L = el otro hijo.
template<typename Container, size_t L>
class InorderIter : public general_iterator<Container, InorderIter<Container, L>> {
public:
    using Node = typename Container::Node;
    using general_iterator<Container, InorderIter<Container, L>>::general_iterator;
    static Node* first(Node* n) { while(n && n->m_pChild[L]) n = n->m_pChild[L]; return n; }
    InorderIter operator++() {
        Node* n = this->m_pNode;
        if(n->m_pChild[!L]) { this->m_pNode = first(n->m_pChild[!L]); return *this; }
        Node* p = n->m_pParent;
        while(p && n == p->m_pChild[!L]) { n = p; p = p->m_pParent; }
        this->m_pNode = p;
        return *this;
    }
};

template<typename Container, size_t L>
class PreorderIter : public general_iterator<Container, PreorderIter<Container, L>> {
public:
    using Node = typename Container::Node;
    using general_iterator<Container, PreorderIter<Container, L>>::general_iterator;
    static Node* first(Node* n) { return n; }
    PreorderIter operator++() {
        Node* n = this->m_pNode;
        if(n->m_pChild[L])  { this->m_pNode = n->m_pChild[L];  return *this; }
        if(n->m_pChild[!L]) { this->m_pNode = n->m_pChild[!L]; return *this; }
        for(Node* p = n->m_pParent; p; n = p, p = p->m_pParent)
            if(n == p->m_pChild[L] && p->m_pChild[!L]) { this->m_pNode = p->m_pChild[!L]; return *this; }
        this->m_pNode = nullptr;
        return *this;
    }
};

template<typename Container, size_t L>
class PostorderIter : public general_iterator<Container, PostorderIter<Container, L>> {
public:
    using Node = typename Container::Node;
    using general_iterator<Container, PostorderIter<Container, L>>::general_iterator;
    static Node* first(Node* n) {
        while(n) {
            if(n->m_pChild[L])       n = n->m_pChild[L];
            else if(n->m_pChild[!L]) n = n->m_pChild[!L];
            else return n;
        }
        return nullptr;
    }
    PostorderIter operator++() {
        Node* n = this->m_pNode, *p = n->m_pParent;
        if(p && n == p->m_pChild[L] && p->m_pChild[!L]) this->m_pNode = first(p->m_pChild[!L]);
        else this->m_pNode = p;
        return *this;
    }
};

// Agrupa begin/end (+rbegin/rend)
template<typename FwdIter, typename RevIter>
struct TreeRange {
    FwdIter m_begin, m_end;
    RevIter m_rbegin, m_rend;
    shared_lock<shared_mutex> m_lock;

    FwdIter begin()  { return m_begin;  }
    FwdIter end()    { return m_end;    }
    RevIter rbegin() { return m_rbegin; }
    RevIter rend()   { return m_rend;   }

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

enum class Traversal { INORDER, PREORDER, POSTORDER };

// ─── BinaryTree ──────────────────────────────────────────────────────────────
template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

    // Reverso exacto via L=0. Identidad: reverso-preorder = espejo-postorder y
    // reverso-postorder = espejo-preorder
    using inorder_fwd   = InorderIter<BinaryTree, 1>;
    using inorder_rev   = InorderIter<BinaryTree, 0>;
    using preorder_fwd  = PreorderIter<BinaryTree, 1>;
    using preorder_rev  = PostorderIter<BinaryTree, 0>;
    using postorder_fwd = PostorderIter<BinaryTree, 1>;
    using postorder_rev = PreorderIter<BinaryTree, 0>;

protected:
    Node*  m_pRoot = nullptr;
    Comp   m_comp;
    mutable shared_mutex m_mtx;

    virtual void internal_insert(Node*& pNode, const value_type& data, Ref ref, Node* parent = nullptr) {
        if(!pNode) { pNode = new Node(data, ref, parent); return; }
        auto branch = !m_comp(pNode->m_data, data);
        internal_insert(pNode->m_pChild[branch], data, ref, pNode);
    }

    virtual void internal_copy(Node*& dst, Node* src, Node* parent = nullptr) {
        if(!src) { dst = nullptr; return; }
        dst = new Node(src->m_data, src->m_ref, parent);
        internal_copy(dst->m_pChild[0], src->m_pChild[0], dst);
        internal_copy(dst->m_pChild[1], src->m_pChild[1], dst);
    }

    virtual void internal_clear(Node* node) {
        if(!node) return;
        internal_clear(node->m_pChild[0]);
        internal_clear(node->m_pChild[1]);
        delete node;
    }

    template<typename Fwd, typename Rev>
    TreeRange<Fwd, Rev> view() {
        shared_lock<shared_mutex> lock(m_mtx);
        return { Fwd(this, Fwd::first(m_pRoot)), Fwd(this, nullptr),
                 Rev(this, Rev::first(m_pRoot)), Rev(this, nullptr), move(lock) };
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

    // 6 recorridos
    auto inorder()    { return view<inorder_fwd,   inorder_rev>();   }
    auto rinorder()   { return view<inorder_rev,   inorder_fwd>();   }
    auto preorder()   { return view<preorder_fwd,  preorder_rev>();  }
    auto rpreorder()  { return view<preorder_rev,  preorder_fwd>();  }
    auto postorder()  { return view<postorder_fwd, postorder_rev>(); }
    auto rpostorder() { return view<postorder_rev, postorder_fwd>(); }

    // begin/end delegan a inorder -> range-based for usa inorder por defecto
    inorder_fwd begin()  { shared_lock<shared_mutex> lock(m_mtx); return inorder_fwd(this, inorder_fwd::first(m_pRoot)); }
    inorder_fwd end()    { return inorder_fwd(this, nullptr); }
    inorder_rev rbegin() { shared_lock<shared_mutex> lock(m_mtx); return inorder_rev(this, inorder_rev::first(m_pRoot)); }
    inorder_rev rend()   { return inorder_rev(this, nullptr); }

    template<typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        for(inorder_fwd it(this, inorder_fwd::first(m_pRoot)), e(this, nullptr); it != e; ++it)
            func(*it, forward<Args>(args)...);
    }

    // toString segun el recorrido; delega en node->toString() (no toca m_data).
    string toString(Traversal order = Traversal::INORDER) const {
        shared_lock<shared_mutex> lock(m_mtx);
        BinaryTree* self = const_cast<BinaryTree*>(this);
        ostringstream oss; oss << "[";
        bool first = true;
        auto dump = [&](auto it, auto e) {
            for(; it != e; ++it) { oss << (first ? "" : ",") << it.getNode()->toString(); first = false; }
        };
        switch(order) {
            case Traversal::INORDER:   dump(inorder_fwd(self, inorder_fwd::first(m_pRoot)),     inorder_fwd(self, nullptr));   break;
            case Traversal::PREORDER:  dump(preorder_fwd(self, preorder_fwd::first(m_pRoot)),   preorder_fwd(self, nullptr));  break;
            case Traversal::POSTORDER: dump(postorder_fwd(self, postorder_fwd::first(m_pRoot)), postorder_fwd(self, nullptr)); break;
        }
        oss << "]";
        return oss.str();
    }

    // Mejora libre #2: printTree por niveles (BFS)
    void printTree(ostream& os) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if(!m_pRoot) { os << "(empty)\n"; return; }
        Vector<Node*> queue;
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
        return os << t.toString();
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
