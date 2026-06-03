#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

//AVLNode
template<typename T>
struct AVLNode : BinaryTreeNode<T, AVLNode<T>> {
    size_t m_height;
    AVLNode(T data, Ref ref): BinaryTreeNode<T, AVLNode<T>>(data, ref), m_height(1) {}
};
template<typename Trait>
class AVL : public BinaryTree<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
private:
    //altura nodo
    size_t  height(Node* n) const {
        if (!n) return 0;
        return n->m_height;
    }
    //update altura
    void update_height(Node* n) {
        if (!n) return;
        n->m_height = 1 + max(height(n->m_pChild[0]), height(n->m_pChild[1]));
    }
    //factorr balance
    size_t  balance_factor(Node* n) const {
        if (!n) return 0;
        return height(n->m_pChild[0]) - height(n->m_pChild[1]);
    }
    //rotar derecha
    void rotate_right(Node* &y) {
        Node* x   = y->m_pChild[0];
        Node* B   = x->m_pChild[1];
        x->m_pChild[1] = y;
        y->m_pChild[0] = B;
        update_height(y);
        update_height(x);
        y = x;
    }
    //rotar izquierda
    void rotate_left(Node* &x) {
        Node* y   = x->m_pChild[1];
        Node* B   = y->m_pChild[0];
        y->m_pChild[0] = x;
        x->m_pChild[1] = B;
        update_height(x);
        update_height(y);
        x = y;
    }
    //rebalanceeo
    void rebalance(Node* &n) {
        update_height(n);
        size_t  bf = balance_factor(n);
        //desbalance izq-izq
        if (bf > 1 && balance_factor(n->m_pChild[0]) >= 0) rotate_right(n);
        //desbalance izq-der
        else if (bf > 1 && balance_factor(n->m_pChild[0])<0) {rotate_left(n->m_pChild[0]);rotate_right(n);}
        //desbalance der-der
        else if (bf < -1 && balance_factor(n->m_pChild[1]) <= 0) rotate_left(n);
        //desbalance der-izq
        else if (bf < -1 && balance_factor(n->m_pChild[1]) > 0) {rotate_right(n->m_pChild[1]);rotate_left(n);}
    }
protected:
    //inserta recur rebalanceo post-order
    void internal_insert(Node* &pNode, const value_type &data, Ref ref) override {
        if (!pNode) { pNode = new AVLNode<value_type>(data, ref); return; }
        auto branch = !this->m_comp(data, pNode->m_data);
        internal_insert(pNode->m_pChild[branch], data, ref);
        rebalance(pNode);
    }
    //internal copy
    Node* internal_copy(Node* pNode) override {
        if (!pNode) return nullptr;
        auto* newNode        = new AVLNode<value_type>(pNode->m_data, pNode->m_ref);
        newNode->m_height    = pNode->m_height;
        newNode->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        newNode->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return newNode;
    }
public:
    AVL() : BinaryTree<Trait>() {}
    //copy constructor
    AVL(const AVL& other) : BinaryTree<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = internal_copy(other.m_pRoot);
    }
    AVL(AVL&& other) : BinaryTree<Trait>(std::move(other)) {}
    AVL& operator=(const AVL& other) {
        if (this != &other) {
            this->clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = internal_copy(other.m_pRoot);
        }
        return *this;
    }
    AVL& operator=(AVL&& other) { BinaryTree<Trait>::operator=(std::move(other)); return *this; }
    virtual ~AVL() {}
    //altura arbol
    size_t height() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return height(this->m_pRoot);
    }
    //factor balance arbol
    size_t balance() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return balance_factor(this->m_pRoot);
    }
};

#endif // __AVL_H__