#ifndef __AVL_H__
#define __AVL_H__

#include <algorithm>
#include "BinaryTree.h"

// Extender BTNode para AVL con campo de altura
template<typename T>
struct AVLNode : public NodeBase<T, AVLNode<T>> {
    using NodeBase<T, AVLNode<T>>::NodeBase;
    T1 m_height = 1;
};

// Hereda BinaryTree
template<typename Trait>
class AVLTree : public BinaryTree<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;

private:
    using Base = BinaryTree<Trait>;

    T1 height(Node* n) const {
        if (!n) return 0;
        return static_cast<AVLNode<value_type>*>(n)->m_height;
    }

    void update_height(Node* n) {
        if (!n) return;
        auto avl_n = static_cast<AVLNode<value_type>*>(n);
        avl_n->m_height = 1 + max(height(n->m_child[0]), height(n->m_child[1]));
    }

    T1 balance_factor(Node* n) const {
        if (!n) return 0;
        return height(n->m_child[0]) - height(n->m_child[1]);
    }

    // Rotacion izquierda
    void rotate_left(Node* &x) {
        Node* y = x->m_child[1];
        Node* B = y->m_child[0];
        y->m_child[0] = x;
        x->m_child[1] = B;
        update_height(x);
        update_height(y);
        x = y;
    }

    // Rotacion derecha
    void rotate_right(Node* &y) {
        Node* x = y->m_child[0];
        Node* B = x->m_child[1];
        x->m_child[1] = y;
        y->m_child[0] = B;
        update_height(y);
        update_height(x);
        y = x;
    }

    // rebalanceo del nodo n
    void rebalance(Node* &n) {
        update_height(n);
        T1 bf = balance_factor(n);
        if      (bf >  1 && balance_factor(n->m_child[0]) >= 0)  rotate_right(n);
        else if (bf >  1 && balance_factor(n->m_child[0]) <  0) { rotate_left (n->m_child[0]); rotate_right(n); }
        else if (bf < -1 && balance_factor(n->m_child[1]) <= 0)  rotate_left (n);
        else if (bf < -1 && balance_factor(n->m_child[1]) >  0) { rotate_right(n->m_child[1]); rotate_left (n); }
    }

protected:
    // internal_insert: delega a BinaryTree y luego rebalancea
    void internal_insert(Node* &node, const value_type &data, Ref ref) override {
        if (!node) { node = new Node(data, ref); return; }
        auto branch = !this->m_cmp(data, node->m_data);
        internal_insert(node->m_child[branch], data, ref);
        rebalance(node);
    }

    // Adaptar
    Node* internal_copy(Node* node) override {
        if (!node) return nullptr;
        Node* n = new Node(node->m_data, node->m_ref);
        n->m_height = node->m_height;
        n->m_child[0] = internal_copy(node->m_child[0]);
        n->m_child[1] = internal_copy(node->m_child[1]);
        return n;
    }

public:
    AVLTree() : BinaryTree<Trait>() {}
    
    virtual ~AVLTree() {}

    // Balance
    T1 balance() const {
        shared_lock<shared_mutex> lock(this->m_lock);
        return balance_factor(this->m_root);
    }
    // Altura del arbol
    T1 height() const {
        shared_lock<shared_mutex> lock(this->m_lock);
        return height(this->m_root);
    }
};

#endif // __AVL_H__