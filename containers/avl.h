#ifndef __AVL_H__
#define __AVL_H__

#include "BinaryTree.h"
#include "../types.h"

// ---------------------------------------------------------------------------
// AVLNode
// ---------------------------------------------------------------------------
template<typename T>
struct AVLNode : BinaryTreeNode<T, AVLNode<T>> {
    T1 m_height;
    AVLNode(T data, Ref ref)
        : BinaryTreeNode<T, AVLNode<T>>(data, ref), m_height(1) {}
};

// ---------------------------------------------------------------------------
// AVL
// ---------------------------------------------------------------------------
template<typename Trait>
class AVL : public BinaryTree<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;

private:

    T1 node_height(Node* n) const {
        if (!n) return 0;
        return n->m_height;
    }

    void update_height(Node* n) {
        if (!n) return;
        n->m_height = 1 + max(node_height(n->m_pChild[0]),
                               node_height(n->m_pChild[1]));
    }

    T1 balance_factor(Node* n) const {
        if (!n) return 0;
        return node_height(n->m_pChild[0]) - node_height(n->m_pChild[1]);
    }

    void rotate_right(Node* &y) {
        Node* x        = y->m_pChild[0];
        Node* B        = x->m_pChild[1];
        x->m_pChild[1] = y;
        y->m_pChild[0] = B;
        update_height(y);   // y es ahora hijo → actualizar primero
        update_height(x);   // x es la nueva raíz → actualizar después
        y = x;
    }

    void rotate_left(Node* &x) {
        Node* y        = x->m_pChild[1];
        Node* B        = y->m_pChild[0];
        y->m_pChild[0] = x;
        x->m_pChild[1] = B;
        update_height(x);   // x es ahora hijo → actualizar primero
        update_height(y);   // y es la nueva raíz → actualizar después
        x = y;
    }


    void rebalance(Node* &n) {
        update_height(n);
        T1 bf = balance_factor(n);

        // Izq-Izq
        if (bf > 1 && balance_factor(n->m_pChild[0]) >= 0) {
            rotate_right(n);
            update_height(n);
        }
        // Izq-Der
        else if (bf > 1 && balance_factor(n->m_pChild[0]) < 0) {
            rotate_left(n->m_pChild[0]);
            rotate_right(n);
            update_height(n);
        }
        // Der-Der
        else if (bf < -1 && balance_factor(n->m_pChild[1]) <= 0) {
            rotate_left(n);
            update_height(n);
        }
        // Der-Izq
        else if (bf < -1 && balance_factor(n->m_pChild[1]) > 0) {
            rotate_right(n->m_pChild[1]);
            rotate_left(n);
            update_height(n);
        }
    }

protected:
    // Inserción recursiva con rebalanceo post-order.

    void internal_insert(Node* &pNode, const value_type &data, Ref ref) override {
        if (!pNode) {
            pNode = new Node(data, ref);
            pNode->m_height = 1;        // asegura altura inicial correcta
            return;
        }
        auto branch = !this->m_comp(data, pNode->m_data);
        internal_insert(pNode->m_pChild[branch], data, ref);
        rebalance(pNode);
    }


    Node* internal_copy(Node* pNode) override {
        if (!pNode) return nullptr;
        auto* newNode        = new Node(pNode->m_data, pNode->m_ref);
        newNode->m_height    = pNode->m_height;
        newNode->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        newNode->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return newNode;
    }

public:
    AVL() : BinaryTree<Trait>() {}

    AVL(const AVL& other) : BinaryTree<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = internal_copy(static_cast<Node*>(other.m_pRoot));
    }

    AVL(AVL&& other) : BinaryTree<Trait>(std::move(other)) {}

    AVL& operator=(const AVL& other) {
        if (this != &other) {
            this->clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = internal_copy(static_cast<Node*>(other.m_pRoot));
        }
        return *this;
    }

    AVL& operator=(AVL&& other) {
        BinaryTree<Trait>::operator=(std::move(other));
        return *this;
    }

    virtual ~AVL() {}


    T1 height() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return node_height(this->m_pRoot);
    }

    T1 balance() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return balance_factor(this->m_pRoot);
    }
};

#endif // __AVL_H__