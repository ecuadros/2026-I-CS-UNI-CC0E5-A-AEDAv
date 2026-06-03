#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <optional>
#include <tuple>
#include <shared_mutex>
#include <utility>
#include "stack.h"
#include "../types.h"
#include "traits.h"

using namespace std;

// ---------------------------------------------------------------------------
// BinaryTreeNode
// ---------------------------------------------------------------------------
template<typename T, typename DerivedNode = void>
struct BinaryTreeNode {
    using value_type = T;
    using Node = std::conditional_t<std::is_void_v<DerivedNode>, BinaryTreeNode, DerivedNode>;
    T    m_data;
    Ref  m_ref;
    Node *m_pChild[2];
    BinaryTreeNode(T data, Ref ref)
        : m_data(data), m_ref(ref), m_pChild{nullptr, nullptr} {}
};

// ---------------------------------------------------------------------------
// BTIteratorBase
// ---------------------------------------------------------------------------
template<typename Node, typename value_type>
class BTIteratorBase {
protected:
    Stack<Node*> m_nodes;
    ptrdiff_t    m_index;
public:
    BTIteratorBase() : m_index(0) {}
    BTIteratorBase(Stack<Node*> s, ptrdiff_t idx) : m_nodes(s), m_index(idx) {}

    value_type& operator*()  const { return  m_nodes[m_index]->m_data; }
    Node*       getNode()    const { return  m_nodes[m_index]; }

    bool operator==(const BTIteratorBase& o) const { return m_index == o.m_index; }
    bool operator!=(const BTIteratorBase& o) const { return !(*this == o); }
};

// ---------------------------------------------------------------------------
// Iteradores diferenciados por Tags
// ---------------------------------------------------------------------------
template<typename Node, typename value_type, typename TraversalTag = void>
class BTForwardIterator : public BTIteratorBase<Node, value_type> {
public:
    using BTIteratorBase<Node, value_type>::BTIteratorBase;
    BTForwardIterator& operator++() { ++this->m_index; return *this; }
};

template<typename Node, typename value_type, typename TraversalTag = void>
class BTBackwardIterator : public BTIteratorBase<Node, value_type> {
public:
    using BTIteratorBase<Node, value_type>::BTIteratorBase;
    BTBackwardIterator& operator++() { --this->m_index; return *this; }
};

// Tags de recorrido
struct InorderTag   {};
struct PreorderTag  {};
struct PostorderTag {};

// ---------------------------------------------------------------------------
// TraversalRange
// ---------------------------------------------------------------------------
template<typename ForwardIt, typename BackwardIt>
class TraversalRange {
    ForwardIt  m_begin;
    ForwardIt  m_end;
    BackwardIt m_rbegin;
    BackwardIt m_rend;
public:
    TraversalRange(ForwardIt b, ForwardIt e, BackwardIt rb, BackwardIt re)
        : m_begin(b), m_end(e), m_rbegin(rb), m_rend(re) {}

    ForwardIt  begin()  const { return m_begin;  }
    ForwardIt  end()    const { return m_end;    }
    BackwardIt rbegin() const { return m_rbegin; }
    BackwardIt rend()   const { return m_rend;   }

    // Itera sobre value_type (el dato)
    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        for (auto it = begin(); it != end(); ++it)
            func(*it, forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    void rForEach(Func func, Args&&... args) {
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it, forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    void forEachNode(Func func, Args&&... args) {
        for (auto it = begin(); it != end(); ++it)
            func(*it.getNode(), forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    void rForEachNode(Func func, Args&&... args) {
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it.getNode(), forward<Args>(args)...);
    }
};

// ---------------------------------------------------------------------------
// BinaryTree
// ---------------------------------------------------------------------------
template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

    // Tipos de iteradores por recorrido
    using InorderForwardIt    = BTForwardIterator <Node, value_type, InorderTag>;
    using InorderBackwardIt   = BTBackwardIterator<Node, value_type, InorderTag>;
    using PreorderForwardIt   = BTForwardIterator <Node, value_type, PreorderTag>;
    using PreorderBackwardIt  = BTBackwardIterator<Node, value_type, PreorderTag>;
    using PostorderForwardIt  = BTForwardIterator <Node, value_type, PostorderTag>;
    using PostorderBackwardIt = BTBackwardIterator<Node, value_type, PostorderTag>;

    // Vistas
    using InorderView   = TraversalRange<InorderForwardIt,   InorderBackwardIt>;
    using PreorderView  = TraversalRange<PreorderForwardIt,  PreorderBackwardIt>;
    using PostorderView = TraversalRange<PostorderForwardIt, PostorderBackwardIt>;

protected:
    Node                *m_pRoot;
    Comp                 m_comp;
    mutable shared_mutex m_mtx;


    virtual void internal_insert(Node* &pNode, const value_type &data, Ref ref) {
        if (!pNode) { pNode = new Node(data, ref); return; }
        auto branch = !m_comp(data, pNode->m_data);
        internal_insert(pNode->m_pChild[branch], data, ref);
    }

    virtual void internal_clear(Node* pNode) {
        if (!pNode) return;
        internal_clear(pNode->m_pChild[0]);
        internal_clear(pNode->m_pChild[1]);
        delete pNode;
    }

    virtual Node* internal_copy(Node* pNode) {
        if (!pNode) return nullptr;
        Node* n        = new Node(pNode->m_data, pNode->m_ref);
        n->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        n->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return n;
    }

    virtual Node* internal_search(Node* pNode, const value_type& data) const {
        if (!pNode) return nullptr;
        if (!m_comp(data, pNode->m_data) && !m_comp(pNode->m_data, data))
            return pNode;
        auto branch = !m_comp(data, pNode->m_data);
        return internal_search(pNode->m_pChild[branch], data);
    }

    virtual size_t internal_size(Node* n) const {
        if (!n) return 0;
        return 1 + internal_size(n->m_pChild[0]) + internal_size(n->m_pChild[1]);
    }

    void fill_inorder(Node* n, Stack<Node*>& s) const {
        if (!n) return;
        fill_inorder(n->m_pChild[0], s);
        s.push(n);
        fill_inorder(n->m_pChild[1], s);
    }

    void fill_preorder(Node* n, Stack<Node*>& s) const {
        if (!n) return;
        s.push(n);
        fill_preorder(n->m_pChild[0], s);
        fill_preorder(n->m_pChild[1], s);
    }

    void fill_postorder(Node* n, Stack<Node*>& s) const {
        if (!n) return;
        fill_postorder(n->m_pChild[0], s);
        fill_postorder(n->m_pChild[1], s);
        s.push(n);
    }

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


    InorderView inorder_nolock() const {
        Stack<Node*> s;
        fill_inorder(m_pRoot, s);
        ptrdiff_t total = (ptrdiff_t)s.size();
        return InorderView(
            InorderForwardIt(s, 0),        InorderForwardIt(s, total),
            InorderBackwardIt(s, total-1), InorderBackwardIt(s, -1));
    }

    PreorderView preorder_nolock() const {
        Stack<Node*> s;
        fill_preorder(m_pRoot, s);
        ptrdiff_t total = (ptrdiff_t)s.size();
        return PreorderView(
            PreorderForwardIt(s, 0),        PreorderForwardIt(s, total),
            PreorderBackwardIt(s, total-1), PreorderBackwardIt(s, -1));
    }

    PostorderView postorder_nolock() const {
        Stack<Node*> s;
        fill_postorder(m_pRoot, s);
        ptrdiff_t total = (ptrdiff_t)s.size();
        return PostorderView(
            PostorderForwardIt(s, 0),        PostorderForwardIt(s, total),
            PostorderBackwardIt(s, total-1), PostorderBackwardIt(s, -1));
    }

public:
    BinaryTree() : m_pRoot(nullptr) {}

    BinaryTree(const BinaryTree& other) : m_pRoot(nullptr) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = internal_copy(other.m_pRoot);
    }

    BinaryTree(BinaryTree&& other) : m_pRoot(nullptr) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
    }

    BinaryTree& operator=(const BinaryTree& other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = internal_copy(other.m_pRoot);
        }
        return *this;
    }

    BinaryTree& operator=(BinaryTree&& other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
        }
        return *this;
    }

    virtual ~BinaryTree() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_clear(m_pRoot);
        m_pRoot = nullptr;
    }

    void insert(const value_type& data, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, data, ref);
    }

    bool contains(const value_type& data) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_search(m_pRoot, data) != nullptr;
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_size(m_pRoot);
    }

    // ------------------------------------------------------------------
    // Vistas públicas (adquieren lock y delegan en _nolock)
    // ------------------------------------------------------------------
    InorderView inorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return inorder_nolock();
    }

    PreorderView preorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return preorder_nolock();
    }

    PostorderView postorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return postorder_nolock();
    }

    // ------------------------------------------------------------------

    InorderForwardIt begin() const {
        return inorder().begin();
    }

    InorderForwardIt end() const {
        return inorder().end();
    }

    InorderBackwardIt rbegin() const {
        return inorder().rbegin();
    }

    InorderBackwardIt rend() const {
        return inorder().rend();
    }

    // ------------------------------------------------------------------
    // toString / operator<<
    // ------------------------------------------------------------------
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s;
        fill_inorder(m_pRoot, s);
        return "Inorder:" + traversalToString(s) + "\n";
    }

    friend ostream& operator<<(ostream& os, const BinaryTree& tree) {
        shared_lock<shared_mutex> lock(tree.m_mtx);
        Stack<Node*> s;
        tree.fill_inorder(tree.m_pRoot, s);
        os << tree.traversalToString(s);
        return os;
    }

    friend istream& operator>>(istream& is, BinaryTree& tree) {
        T2 ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type val; Ref ref; T2 comma, paren;
        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref >> paren) {
                    if (comma == ',' && paren == ')')
                        tree.insert(val, ref);
                }
            }
        }
        return is;
    }


    void printTree(ostream& os = cout) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) { os << "(arbol vacio)" << endl; return; }

        Vector<Node*> queue(64);
        size_t front = 0;
        queue.push_back(m_pRoot,0);      

        while (front < queue.size()) {
            size_t level_size = queue.size() - front;
            bool   all_null   = true;
            for (size_t i = 0; i < level_size; ++i) {
                if (queue[front + i]) { all_null = false; break; }
            }
            if (all_null) break;

            os << "  ";
            for (size_t i = 0; i < level_size; ++i) {
                Node* n = queue[front++];
                if (n) {
                    os << n->m_data;
                    queue.push_back(n->m_pChild[0],0);
                    queue.push_back(n->m_pChild[1],0); 
                } else {
                    os << "_";
                    queue.push_back(nullptr,0);         
                    queue.push_back(nullptr,0);         
                }
                os << " ";
            }
            os << endl;
        }
    }
};

#endif // __BINARYTREE_H__