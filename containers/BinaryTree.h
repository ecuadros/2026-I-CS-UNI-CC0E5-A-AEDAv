#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <stack>
#include <queue>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <limits>
#include <functional>
#include <type_traits>

#include "general_iterator.h"
#include "traits.h"
#include "../types.h"

using namespace std;

template<typename T, typename NodeType = void>
struct BinaryTreeNode {
    using value_type = T;
protected:
    using Node = typename conditional<is_same<NodeType, void>::value,
                                      BinaryTreeNode<T, void>, NodeType>::type;
public:
    T     m_data;
    Ref   m_ref;
    Node* m_pChild[2];
    BinaryTreeNode() : m_data(T()), m_ref(Ref()), m_pChild{nullptr, nullptr} {}
    BinaryTreeNode(T data, Ref ref) : m_data(data), m_ref(ref), m_pChild{nullptr, nullptr} {}
    virtual ~BinaryTreeNode() {}
    T&  getDataRef()    { return m_data; }
    T   getData() const { return m_data; }
    Ref getRef()  const { return m_ref; }
};

template <typename T>
struct AscendingBTTrait  : BaseTrait<T, less<T>,    BinaryTreeNode<T>> {};
template <typename T>
struct DescendingBTTrait : BaseTrait<T, greater<T>, BinaryTreeNode<T>> {};

template <typename Trait> class BinaryTree;

// ToDo forward iterator (inorder)
template <typename Container>
class bt_inorder_fwd : public general_iterator<Container, bt_inorder_fwd<Container>> {
    using Node = typename Container::Node;
    stack<Node*> m_st;
    void pushLeft(Node* n) { while (n) { m_st.push(n); n = n->m_pChild[0]; } }
public:
    using MySelf = bt_inorder_fwd<Container>;
    using Parent = general_iterator<Container, MySelf>;
    bt_inorder_fwd(Container* c, Node* root) : Parent(c, nullptr) {
        pushLeft(root);
        if (!m_st.empty()) this->m_pNode = m_st.top();
    }
    MySelf operator++() {
        if (!m_st.empty()) {
            Node* n = m_st.top(); m_st.pop();
            pushLeft(n->m_pChild[1]);
            this->m_pNode = m_st.empty() ? nullptr : m_st.top();
        }
        return *this;
    }
};

// ToDo backward iterator (inorder)
template <typename Container>
class bt_inorder_bwd : public general_iterator<Container, bt_inorder_bwd<Container>> {
    using Node = typename Container::Node;
    stack<Node*> m_st;
    void pushRight(Node* n) { while (n) { m_st.push(n); n = n->m_pChild[1]; } }
public:
    using MySelf = bt_inorder_bwd<Container>;
    using Parent = general_iterator<Container, MySelf>;
    bt_inorder_bwd(Container* c, Node* root) : Parent(c, nullptr) {
        pushRight(root);
        if (!m_st.empty()) this->m_pNode = m_st.top();
    }
    MySelf operator++() {
        if (!m_st.empty()) {
            Node* n = m_st.top(); m_st.pop();
            pushRight(n->m_pChild[0]);
            this->m_pNode = m_st.empty() ? nullptr : m_st.top();
        }
        return *this;
    }
};

// ToDo forward iterator (preorder)
template <typename Container>
class bt_preorder_fwd : public general_iterator<Container, bt_preorder_fwd<Container>> {
    using Node = typename Container::Node;
    stack<Node*> m_st;
public:
    using MySelf = bt_preorder_fwd<Container>;
    using Parent = general_iterator<Container, MySelf>;
    bt_preorder_fwd(Container* c, Node* root) : Parent(c, nullptr) {
        if (root) { m_st.push(root); this->m_pNode = root; }
    }
    MySelf operator++() {
        if (!m_st.empty()) {
            Node* n = m_st.top(); m_st.pop();
            if (n->m_pChild[1]) m_st.push(n->m_pChild[1]);
            if (n->m_pChild[0]) m_st.push(n->m_pChild[0]);
            this->m_pNode = m_st.empty() ? nullptr : m_st.top();
        }
        return *this;
    }
};

// ToDo backward iterator (preorder)
template <typename Container>
class bt_preorder_bwd : public general_iterator<Container, bt_preorder_bwd<Container>> {
    using Node = typename Container::Node;
    stack<Node*> m_st;
public:
    using MySelf = bt_preorder_bwd<Container>;
    using Parent = general_iterator<Container, MySelf>;
    bt_preorder_bwd(Container* c, Node* root) : Parent(c, nullptr) {
        if (root) { m_st.push(root); this->m_pNode = root; }
    }
    MySelf operator++() {
        if (!m_st.empty()) {
            Node* n = m_st.top(); m_st.pop();
            if (n->m_pChild[0]) m_st.push(n->m_pChild[0]);
            if (n->m_pChild[1]) m_st.push(n->m_pChild[1]);
            this->m_pNode = m_st.empty() ? nullptr : m_st.top();
        }
        return *this;
    }
};

// ToDo forward iterator (postorder)
template <typename Container>
class bt_postorder_fwd : public general_iterator<Container, bt_postorder_fwd<Container>> {
    using Node = typename Container::Node;
    deque<Node*> m_order;
    size_t       m_idx = 0;
public:
    using MySelf = bt_postorder_fwd<Container>;
    using Parent = general_iterator<Container, MySelf>;
    bt_postorder_fwd(Container* c, Node* root) : Parent(c, nullptr) {
        if (root) {
            stack<Node*> s1, s2;
            s1.push(root);
            while (!s1.empty()) {
                Node* n = s1.top(); s1.pop(); s2.push(n);
                if (n->m_pChild[0]) s1.push(n->m_pChild[0]);
                if (n->m_pChild[1]) s1.push(n->m_pChild[1]);
            }
            while (!s2.empty()) { m_order.push_back(s2.top()); s2.pop(); }
            this->m_pNode = m_order.front();
        }
    }
    MySelf operator++() {
        ++m_idx;
        this->m_pNode = (m_idx < m_order.size()) ? m_order[m_idx] : nullptr;
        return *this;
    }
};

// ToDo backward iterator (postorder)
template <typename Container>
class bt_postorder_bwd : public general_iterator<Container, bt_postorder_bwd<Container>> {
    using Node = typename Container::Node;
    deque<Node*> m_order;
    size_t       m_idx = 0;
public:
    using MySelf = bt_postorder_bwd<Container>;
    using Parent = general_iterator<Container, MySelf>;
    bt_postorder_bwd(Container* c, Node* root) : Parent(c, nullptr) {
        if (root) {
            stack<Node*> s1, s2;
            s1.push(root);
            while (!s1.empty()) {
                Node* n = s1.top(); s1.pop(); s2.push(n);
                if (n->m_pChild[1]) s1.push(n->m_pChild[1]);
                if (n->m_pChild[0]) s1.push(n->m_pChild[0]);
            }
            while (!s2.empty()) { m_order.push_back(s2.top()); s2.pop(); }
            this->m_pNode = m_order.front();
        }
    }
    MySelf operator++() {
        ++m_idx;
        this->m_pNode = (m_idx < m_order.size()) ? m_order[m_idx] : nullptr;
        return *this;
    }
};

// ToDo Mejora #1: BFS - recorre el arbol nivel por nivel usando queue
template <typename Container>
class bt_levelorder_fwd : public general_iterator<Container, bt_levelorder_fwd<Container>> {
    using Node = typename Container::Node;
    queue<Node*> m_q;
public:
    using MySelf = bt_levelorder_fwd<Container>;
    using Parent = general_iterator<Container, MySelf>;
    bt_levelorder_fwd(Container* c, Node* root) : Parent(c, nullptr) {
        if (root) { m_q.push(root); this->m_pNode = root; }
    }
    MySelf operator++() {
        if (!m_q.empty()) {
            Node* n = m_q.front(); m_q.pop();
            if (n->m_pChild[0]) m_q.push(n->m_pChild[0]);
            if (n->m_pChild[1]) m_q.push(n->m_pChild[1]);
            this->m_pNode = m_q.empty() ? nullptr : m_q.front();
        }
        return *this;
    }
};

template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = BinaryTree<Trait>;

    using forward_iterator            = bt_inorder_fwd<MySelf>;
    using backward_iterator           = bt_inorder_bwd<MySelf>;
    using preorder_forward_iterator   = bt_preorder_fwd<MySelf>;
    using preorder_backward_iterator  = bt_preorder_bwd<MySelf>;
    using postorder_forward_iterator  = bt_postorder_fwd<MySelf>;
    using postorder_backward_iterator = bt_postorder_bwd<MySelf>;
    using levelorder_iterator         = bt_levelorder_fwd<MySelf>;

    friend forward_iterator;
    friend backward_iterator;
    friend preorder_forward_iterator;
    friend preorder_backward_iterator;
    friend postorder_forward_iterator;
    friend postorder_backward_iterator;
    friend levelorder_iterator;

protected:
    Node*                m_pRoot = nullptr;
    size_t               m_size  = 0;
    Comp                 m_comp;
    mutable shared_mutex m_mtx;  // ToDo Concurrency

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->m_pChild[0]);
        destroy(n->m_pChild[1]);
        delete n;
    }

    Node* clone(Node* src) {
        if (!src) return nullptr;
        Node* n = new Node(src->m_data, src->m_ref);
        n->m_pChild[0] = clone(src->m_pChild[0]);
        n->m_pChild[1] = clone(src->m_pChild[1]);
        return n;
    }

    virtual void internal_insert(Node*& pNode, const value_type& data, Ref ref) {
        if (!pNode) { pNode = new Node(data, ref); m_size++; return; }
        auto branch = !m_comp(pNode->m_data, data);
        internal_insert(pNode->m_pChild[branch], data, ref);
    }

    size_t height_r(Node* n) const {
        if (!n) return 0;
        return 1 + max(height_r(n->m_pChild[0]), height_r(n->m_pChild[1]));
    }

    // ToDo Mejora #2: print2D - imprime el arbol rotado 90 grados (derecho=arriba, izquierdo=abajo)
    void print2D_r(ostream& os, Node* n, int depth) const {
        if (!n) return;
        print2D_r(os, n->m_pChild[1], depth + 1);
        for (int i = 0; i < depth * 4; ++i) os << ' ';
        os << n->m_data << '\n';
        print2D_r(os, n->m_pChild[0], depth + 1);
    }

public:
    BinaryTree() : m_pRoot(nullptr), m_size(0) {}

    // ToDo Constructor copia
    BinaryTree(const BinaryTree& other) : m_pRoot(nullptr), m_size(0) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = clone(other.m_pRoot);
        m_size  = other.m_size;
    }

    // ToDo Move Constructor
    BinaryTree(BinaryTree&& other) noexcept : m_pRoot(nullptr), m_size(0) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = exchange(other.m_pRoot, nullptr);
        m_size  = exchange(other.m_size,  0);
    }

    // ToDo Destructor Seguro
    virtual ~BinaryTree() {
        unique_lock<shared_mutex> lock(m_mtx);
        destroy(m_pRoot);
        m_pRoot = nullptr;
    }

    BinaryTree& operator=(const BinaryTree& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lockSelf(m_mtx);
            destroy(m_pRoot); m_pRoot = nullptr; m_size = 0;
            shared_lock<shared_mutex> lockOther(other.m_mtx);
            m_pRoot = clone(other.m_pRoot);
            m_size  = other.m_size;
        }
        return *this;
    }

    BinaryTree& operator=(BinaryTree&& other) noexcept {
        if (this != &other) {
            unique_lock<shared_mutex> lockSelf(m_mtx);
            destroy(m_pRoot); m_pRoot = nullptr; m_size = 0;
            unique_lock<shared_mutex> lockOther(other.m_mtx);
            m_pRoot = exchange(other.m_pRoot, nullptr);
            m_size  = exchange(other.m_size,  0);
        }
        return *this;
    }

    virtual void insert(const value_type& data, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, data, ref);
    }

    size_t size()   const { shared_lock<shared_mutex> lock(m_mtx); return m_size; }
    size_t height() const { shared_lock<shared_mutex> lock(m_mtx); return height_r(m_pRoot); }

    forward_iterator  begin()  { return forward_iterator(this, m_pRoot); }
    forward_iterator  end()    { return forward_iterator(this, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, m_pRoot); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    preorder_forward_iterator  pre_begin()  { return preorder_forward_iterator(this, m_pRoot); }
    preorder_forward_iterator  pre_end()    { return preorder_forward_iterator(this, nullptr); }
    preorder_backward_iterator pre_rbegin() { return preorder_backward_iterator(this, m_pRoot); }
    preorder_backward_iterator pre_rend()   { return preorder_backward_iterator(this, nullptr); }

    postorder_forward_iterator  post_begin()  { return postorder_forward_iterator(this, m_pRoot); }
    postorder_forward_iterator  post_end()    { return postorder_forward_iterator(this, nullptr); }
    postorder_backward_iterator post_rbegin() { return postorder_backward_iterator(this, m_pRoot); }
    postorder_backward_iterator post_rend()   { return postorder_backward_iterator(this, nullptr); }

    levelorder_iterator level_begin() { return levelorder_iterator(this, m_pRoot); }
    levelorder_iterator level_end()   { return levelorder_iterator(this, nullptr); }

    // ToDo ToString
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "[";
        bool first = true;
        stack<Node*> st;
        Node* n = m_pRoot;
        while (n || !st.empty()) {
            while (n) { st.push(n); n = n->m_pChild[0]; }
            n = st.top(); st.pop();
            if (!first) oss << ",";
            oss << "(" << n->m_data << "," << n->m_ref << ")";
            first = false;
            n = n->m_pChild[1];
        }
        oss << "]";
        return oss.str();
    }

    // ToDo usar en un bucle nativo foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        shared_lock<shared_mutex> lock(m_mtx);
        stack<Node*> st;
        Node* n = m_pRoot;
        while (n || !st.empty()) {
            while (n) { st.push(n); n = n->m_pChild[0]; }
            n = st.top(); st.pop();
            func(n->m_data, forward<Args>(args)...);
            n = n->m_pChild[1];
        }
    }

    void print2D(ostream& os = cout) const {
        shared_lock<shared_mutex> lock(m_mtx);
        print2D_r(os, m_pRoot, 0);
    }

    // ToDo operator<< (incluye persistencia a archivos)
    friend ostream& operator<<(ostream& os, const BinaryTree& t) {
        shared_lock<shared_mutex> lock(t.m_mtx);
        os << "[";
        bool first = true;
        stack<Node*> st;
        Node* n = t.m_pRoot;
        while (n || !st.empty()) {
            while (n) { st.push(n); n = n->m_pChild[0]; }
            n = st.top(); st.pop();
            if (!first) os << ",";
            os << "(" << n->m_data << "," << n->m_ref << ")";
            first = false;
            n = n->m_pChild[1];
        }
        os << "]";
        return os;
    }

    // ToDo operator>>
    friend istream& operator>>(istream& is, BinaryTree& t) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type val;
        Ref ref;
        char comma, parenClose;
        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref >> parenClose)
                    if (comma == ',' && parenClose == ')')
                        t.insert(val, ref);
            }
        }
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }
};

#endif // __BINARYTREE_H__
