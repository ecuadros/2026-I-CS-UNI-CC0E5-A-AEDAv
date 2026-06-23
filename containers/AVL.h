#ifndef __AVL_H__
#define __AVL_H__

#include <algorithm>
#include <shared_mutex>

#include "BinaryTree.h"
#include "traits.h"
#include "../types.h"

using namespace std;

// ToDo Extender el BinaryTreeNode para tener la altura
template <typename T>
struct AVLNode : public BinaryTreeNode<T, AVLNode<T>> {
    using Base = BinaryTreeNode<T, AVLNode<T>>;
    size_t m_height = 1;

    AVLNode() : Base() {}
    AVLNode(T data, Ref ref) : Base(data, ref) {}

    size_t getHeight() const   { return m_height; }
    void   setHeight(size_t h) { m_height = h; }
};

template <typename T>
struct AscendingAVLTrait  : BaseTrait<T, less<T>,    AVLNode<T>> {};
template <typename T>
struct DescendingAVLTrait : BaseTrait<T, greater<T>, AVLNode<T>> {};

// ToDo Adaptar insert de BinaryTree
template <typename Trait>
class AVL : public BinaryTree<Trait> {
public:
    using Base       = BinaryTree<Trait>;
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using MySelf     = AVL<Trait>;

private:
    size_t node_height(Node* n) const { return n ? n->getHeight() : 0; }

    Balance balance_factor(Node* n) const {
        if (!n) return 0;
        return (Balance)node_height(n->m_pChild[0]) - (Balance)node_height(n->m_pChild[1]);
    }

    void update_height(Node* n) {
        if (n) n->setHeight(1 + max(node_height(n->m_pChild[0]),
                                    node_height(n->m_pChild[1])));
    }

    Node* rotateRight(Node* y) {
        Node* x  = y->m_pChild[0];
        Node* t2 = x->m_pChild[1];
        x->m_pChild[1] = y;
        y->m_pChild[0] = t2;
        update_height(y);
        update_height(x);
        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y  = x->m_pChild[1];
        Node* t2 = y->m_pChild[0];
        y->m_pChild[0] = x;
        x->m_pChild[1] = t2;
        update_height(x);
        update_height(y);
        return y;
    }

    void balance(Node*& n) {
        if (!n) return;
        update_height(n);
        Balance bf = balance_factor(n);
        if (bf > 1) {
            if (balance_factor(n->m_pChild[0]) < 0)
                n->m_pChild[0] = rotateLeft(n->m_pChild[0]);
            n = rotateRight(n);
        } else if (bf < -1) {
            if (balance_factor(n->m_pChild[1]) > 0)
                n->m_pChild[1] = rotateRight(n->m_pChild[1]);
            n = rotateLeft(n);
        }
    }

protected:
    void internal_insert(Node*& pNode, const value_type& data, Ref ref) override {
        Base::internal_insert(pNode, data, ref);
        balance(pNode);
    }

public:
    AVL() : Base() {}

    size_t height() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return node_height(this->m_pRoot);
    }

    Balance balanceFactor() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return balance_factor(this->m_pRoot);
    }
};

#endif // __AVL_H__
