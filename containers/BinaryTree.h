#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <tuple>
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
#include "traits.h"
using namespace std;

// Binary Tree Node
template <typename T>
class BinaryTreeNode{
public:
    using value_type = T;
    using Node       = BinaryTreeNode<T>;
private:
    T     m_data;
    Ref   m_ref;
    Node *m_pParent;
    Node *m_pChild[2]; // 0 = left, 1 = right
public:
    BinaryTreeNode() : m_data(T()), m_ref(Ref()), m_pParent(nullptr), m_pChild{nullptr, nullptr} {}
    BinaryTreeNode(T data, Ref ref) : m_data(data), m_ref(ref), m_pParent(nullptr), m_pChild{nullptr, nullptr} {}
    BinaryTreeNode(T data, Ref ref, Node *parent) : m_data(data), m_ref(ref), m_pParent(parent), m_pChild{nullptr, nullptr} {}
    virtual ~BinaryTreeNode() {}

    T      getData() const                  { return m_data; }
    T&     getDataRef()                     { return m_data; }
    void   setData(T data)                  { m_data = data; }
    Ref    getRef() const                   { return m_ref; }
    void   setRef(Ref ref)                  { m_ref = ref; }

    Node*  getChild(size_t branch) const    { return m_pChild[branch]; }
    Node*& getChildRef(size_t branch)       { return m_pChild[branch]; }
    void   setChild(size_t branch, Node *c) { m_pChild[branch] = c; }
    Node*  getLeft()  const                 { return m_pChild[0]; }
    Node*  getRight() const                 { return m_pChild[1]; }

    Node*  getParent() const                { return m_pParent; }
    void   setParent(Node *parent)          { m_pParent = parent; }
};

// Traits de Ordenamiento (mismo patron que AscendingLinkedListTrait)
template <typename T>
struct AscendingBTTrait : public BaseTrait<BinaryTreeNode<T>, less<T>>{
};

template <typename T>
struct DescendingBTTrait : public BaseTrait<BinaryTreeNode<T>, greater<T>>{
};


// Iterador Forward Inorder
template <typename Container>
class BTInorderForwardIterator : public general_iterator<Container, BTInorderForwardIterator<Container>>{
public:
    using MySelf = BTInorderForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    using Parent::Parent;

    MySelf operator++() {
        Node *n = this->m_pNode;
        if (n) {
            if (n->getRight()){
                n = n->getRight();
                while (n->getLeft()) n = n->getLeft();
            } else {
                Node *p = n->getParent();
                while (p && n == p->getRight()) {
                    n = p;
                    p = p->getParent();
                }
                n = p;
            }
            this->m_pNode = n;
        }
        return *this;
    }
};

// Iterador Backward Inorder
template <typename Container>
class BTInorderBackwardIterator : public general_iterator<Container, BTInorderBackwardIterator<Container>>{
public:
    using MySelf = BTInorderBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    using Parent::Parent;

    MySelf operator++() {
        Node *n = this->m_pNode;
        if (n) {
            if (n->getLeft()){
                n = n->getLeft();
                while (n->getRight()) n = n->getRight();
            } else {
                Node *p = n->getParent();
                while (p && n == p->getLeft()) {
                    n = p;
                    p = p->getParent();
                }
                n = p;
            }
            this->m_pNode = n;
        }
        return *this;
    }
};

// Iterador Forward Preorder
template <typename Container>
class BTPreorderForwardIterator : public general_iterator<Container, BTPreorderForwardIterator<Container>>{
public:
    using MySelf = BTPreorderForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    using Parent::Parent;

    MySelf operator++(){
        Node *n = this->m_pNode;
        if (n){
            if (n->getLeft()){
                this->m_pNode = n->getLeft();
                return *this;
            }
            if (n->getRight()){
                this->m_pNode = n->getRight();
                return *this;
            }
            Node *p = n->getParent();
            while (p){
                if (n == p->getLeft() && p->getRight()) {
                    this->m_pNode = p->getRight();
                    return *this;
                }
                n = p;
                p = p->getParent();
            }
            this->m_pNode = nullptr;
        }
        return *this;
    }
};

// Iterador Backward Preorder
template <typename Container>
class BTPreorderBackwardIterator : public general_iterator<Container, BTPreorderBackwardIterator<Container>>{
public:
    using MySelf = BTPreorderBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    using Parent::Parent;

    MySelf operator++(){
        Node *n = this->m_pNode;
        if (n){
            Node *p = n->getParent();
            if (!p){
                this->m_pNode = nullptr;
                return *this;
            }
            if (n == p->getRight() && p->getLeft()){
                // preorder_last del subarbol izquierdo del padre
                Node *m = p->getLeft();
                while (m->getLeft() || m->getRight())
                    m = m->getRight() ? m->getRight() : m->getLeft();
                this->m_pNode = m;
            } else {
                this->m_pNode = p;
            }
        }
        return *this;
    }
};

// Iterador Forward Postorder
template <typename Container>
class BTPostorderForwardIterator : public general_iterator<Container, BTPostorderForwardIterator<Container>>{
public:
    using MySelf = BTPostorderForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    using Parent::Parent;

    MySelf operator++(){
        Node *n = this->m_pNode;
        if (n) {
            Node *p = n->getParent();
            if (!p){
                this->m_pNode = nullptr;
                return *this;
            }
            if (n == p->getLeft() && p->getRight()){
                // postorder_first del subarbol derecho del padre
                Node *m = p->getRight();
                while (m->getLeft() || m->getRight())
                    m = m->getLeft() ? m->getLeft() : m->getRight();
                this->m_pNode = m;
            } else {
                this->m_pNode = p;
            }
        }
        return *this;
    }
};

// Iterador Backward Postorder
template <typename Container>
class BTPostorderBackwardIterator : public general_iterator<Container, BTPostorderBackwardIterator<Container>>{
public:
    using MySelf = BTPostorderBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    using Parent::Parent;

    MySelf operator++(){
        Node *n = this->m_pNode;
        if (n){
            if (n->getRight()){
                this->m_pNode = n->getRight();
                return *this;
            }
            if (n->getLeft()){
                this->m_pNode = n->getLeft();
                return *this;
            }
            Node *p = n->getParent();
            while (p) {
                if (n == p->getRight() && p->getLeft()){
                    this->m_pNode = p->getLeft();
                    return *this;
                }
                n = p;
                p = p->getParent();
            }
            this->m_pNode = nullptr;
        }
        return *this;
    }
};


// BinaryTree
template <typename Trait>
class BinaryTree{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = BinaryTree<Trait>;

    using inorder_iterator             = BTInorderForwardIterator<MySelf>;
    using inorder_reverse_iterator     = BTInorderBackwardIterator<MySelf>;
    using preorder_iterator            = BTPreorderForwardIterator<MySelf>;
    using preorder_reverse_iterator    = BTPreorderBackwardIterator<MySelf>;
    using postorder_iterator           = BTPostorderForwardIterator<MySelf>;
    using postorder_reverse_iterator   = BTPostorderBackwardIterator<MySelf>;

    // begin/end por defecto = inorder forward
    using forward_iterator             = inorder_iterator;
    using backward_iterator            = inorder_reverse_iterator;

    friend inorder_iterator;
    friend inorder_reverse_iterator;
    friend preorder_iterator;
    friend preorder_reverse_iterator;
    friend postorder_iterator;
    friend postorder_reverse_iterator;

protected:
    Node   *m_pRoot = nullptr;
    size_t  m_size  = 0;
    Comp    m_comp;
    mutable shared_mutex m_mtx;

    virtual void  internal_insert(Node *&pNode, Node *parent, const value_type &value, Ref ref);
    virtual Node* internal_clone(Node *src, Node *parent);
    void  internal_destroy(Node *n);
    void  internal_dump(ostream &os, Node *n, bool &first) const;

public:
    BinaryTree() {}

    // Copy Constructor
    BinaryTree(const BinaryTree &other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = internal_clone(other.m_pRoot, nullptr);
        m_size  = other.m_size;
    }


    // Move Constructor
    BinaryTree(BinaryTree &&other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_size  = std::exchange(other.m_size,  0);
    }


    // Copy Assignment
    BinaryTree& operator=(const BinaryTree &other) {
        if (this != &other) {
            unique_lock<shared_mutex> lockMe(m_mtx);
            internal_destroy(m_pRoot);
            m_pRoot = nullptr;
            m_size  = 0;
            shared_lock<shared_mutex> lockOther(other.m_mtx);
            m_pRoot = internal_clone(other.m_pRoot, nullptr);
            m_size  = other.m_size;
        }
        return *this;
    }

    // Move Assignment
    BinaryTree& operator=(BinaryTree &&other) {
        if (this != &other) {
            unique_lock<shared_mutex> lockMe(m_mtx);
            internal_destroy(m_pRoot);
            unique_lock<shared_mutex> lockOther(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    // Destructor Seguro
    virtual ~BinaryTree() {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_destroy(m_pRoot);
        m_pRoot = nullptr;
        m_size  = 0;
    }

    // Operaciones
    virtual void   insert(const value_type &value, Ref ref);
    virtual size_t size() const;

    // Iteradores
    inorder_iterator begin() {
        Node *n = m_pRoot;
        while (n && n->getLeft()) {
            n = n->getLeft();
        }
        return inorder_iterator(this, n);
    }
    inorder_iterator end() {return inorder_iterator(this, nullptr);}
    inorder_reverse_iterator rbegin() {
        Node *n = m_pRoot;
        while (n && n->getRight()) {
            n = n->getRight();
        }
        return inorder_reverse_iterator(this, n);
    }
    inorder_reverse_iterator rend() {return inorder_reverse_iterator(this, nullptr);}

    preorder_iterator pre_begin() {return preorder_iterator(this, m_pRoot);}
    preorder_iterator pre_end() {return preorder_iterator(this, nullptr);}

    preorder_reverse_iterator pre_rbegin() {
        Node *n = m_pRoot;
        while (n && (n->getLeft() || n->getRight())) {
            n = n->getRight() ? n->getRight() : n->getLeft();
        }
        return preorder_reverse_iterator(this, n);
    }
    preorder_reverse_iterator pre_rend() {return preorder_reverse_iterator(this, nullptr);}

    postorder_iterator post_begin() {
        Node *n = m_pRoot;
        while (n && (n->getLeft() || n->getRight())) {
            n = n->getLeft() ? n->getLeft() : n->getRight();
        }
        return postorder_iterator(this, n);
    }
    postorder_iterator post_end() {return postorder_iterator(this, nullptr);}

    postorder_reverse_iterator post_rbegin() {return postorder_reverse_iterator(this, m_pRoot);}
    postorder_reverse_iterator post_rend() {return postorder_reverse_iterator(this, nullptr);}


    // ForEach  begin()/end() son inorder, asi que recorre inorder.
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        for (auto& item : *this) {
            func(item, std::forward<Args>(args)...);
        }
    }

    // Operadores I/O
    friend ostream& operator<<(ostream& os, const BinaryTree& tree){
        shared_lock<shared_mutex> lock(tree.m_mtx);
        os << "[";
        bool first = true;
        tree.internal_dump(os, tree.m_pRoot, first);
        os << "]";
        return os;
    }

    friend istream& operator>>(istream& is, BinaryTree& tree) {
        char ch;
        if (!(is >> ch) || ch != '['){
            is.clear(ios_base::failbit);
            return is;
        }
        value_type val;
        Ref ref;
        char comma, parenClose;
        while (is >> ch && ch != ']'){
            if (ch == '(') {
                if (is >> val >> comma >> ref >> parenClose) {
                    if (comma == ',' && parenClose == ')') {
                        tree.insert(val, ref);
                    }
                }
            }
        }
        return is;
    }
};

// Implementacion de Metodos del Arbol
template <typename Trait>
void BinaryTree<Trait>::internal_insert(Node *&pNode, Node *parent, const value_type &value, Ref ref){
    if (pNode == nullptr) {
        pNode = new Node(value, ref, parent);
        m_size++;
        return;
    }
    auto branch = !m_comp(value, pNode->getData());
    internal_insert(pNode->getChildRef(branch), pNode, value, ref);
}

template <typename Trait>
void BinaryTree<Trait>::insert(const value_type &value, Ref ref) {
    unique_lock<shared_mutex> lock(m_mtx);
    internal_insert(m_pRoot, nullptr, value, ref);
}

template <typename Trait>
size_t BinaryTree<Trait>::size() const {
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size;
}


template <typename Trait>
typename BinaryTree<Trait>::Node*
BinaryTree<Trait>::internal_clone(Node *src, Node *parent) {
    if (!src) return nullptr;
    Node *n = new Node(src->getData(), src->getRef(), parent);
    n->setChild(0, internal_clone(src->getLeft(),  n));
    n->setChild(1, internal_clone(src->getRight(), n));
    return n;
}

template <typename Trait>
void BinaryTree<Trait>::internal_destroy(Node *n) {
    if (!n) return;
    internal_destroy(n->getLeft());
    internal_destroy(n->getRight());
    delete n;
}

template <typename Trait>
void BinaryTree<Trait>::internal_dump(ostream &os, Node *n, bool &first) const {
    if (!n) return;
    internal_dump(os, n->getLeft(), first);
    if (!first) os << ",";
    os << "(" << n->getData() << "," << n->getRef() << ")";
    first = false;
    internal_dump(os, n->getRight(), first);
}

#endif

