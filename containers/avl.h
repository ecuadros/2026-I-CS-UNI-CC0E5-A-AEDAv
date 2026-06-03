#ifndef __AVL_H__
#define __AVL_H__
#include "BinaryTree.h"
using namespace std;

// ─── AVLNode (CRTP) ──────────────────────────────────────────────────────────
template<typename T>
struct AVLNode : public BinaryTreeNode<T, AVLNode<T>> {
    size_t m_height = 1;
    AVLNode(T data, Ref ref) : BinaryTreeNode<T, AVLNode<T>>(data, ref) {}
};

// Traits
template<typename T>
struct AscendingAVLTrait  : BaseTrait<AVLNode<T>, less<T>>    {};
template<typename T>
struct DescendingAVLTrait : BaseTrait<AVLNode<T>, greater<T>> {};

// ─── AVL ─────────────────────────────────────────────────────────────────────
template<typename Trait>
class AVL : public BinaryTree<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using BinaryTree<Trait>::BinaryTree;

    // Copy constructor propio: llama al vtable de AVL -> internal_copy preserva m_height
    AVL(const AVL& other) : BinaryTree<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        internal_copy(this->m_pRoot, other.m_pRoot);
    }

    // Move real: delega en BinaryTree (declarar copy ctor elimina el move implicito)
    AVL(AVL&& other) noexcept : BinaryTree<Trait>(move(other)) {}

private:
    size_t    height(Node* n)        { return n ? n->m_height : 0; }

    // bf = height(left) - height(right) -> left=m_pChild[1], right=m_pChild[0]
    ptrdiff_t balance_factor(Node* n) { return (ptrdiff_t)height(n->m_pChild[1]) - (ptrdiff_t)height(n->m_pChild[0]); }

    void update_height(Node* n) {
        n->m_height = 1 + max(height(n->m_pChild[0]), height(n->m_pChild[1]));
    }

    void rotate(Node*& n, size_t dir) {
        size_t other           = 1 - dir;
        Node* child            = n->m_pChild[dir];
        n->m_pChild[dir]       = child->m_pChild[other];
        child->m_pChild[other] = n;
        update_height(n);
        update_height(child);
        n = child;
    }

    void rebalance(Node*& n) {
        update_height(n);
        ptrdiff_t bf = balance_factor(n);
        if(bf > 1) {                                  // left-heavy
            if(balance_factor(n->m_pChild[1]) < 0)
                rotate(n->m_pChild[1], 0);            // LR: rotar izq primero
            rotate(n, 1);                             // LL (o LR ya corregido)
        } else if(bf < -1) {                          // right-heavy
            if(balance_factor(n->m_pChild[0]) > 0)
                rotate(n->m_pChild[0], 1);            // RL: rotar der primero
            rotate(n, 0);                             // RR (o RL ya corregido)
        }
    }

protected:
    void internal_insert(Node*& pNode, const value_type& data, Ref ref) override {
        BinaryTree<Trait>::internal_insert(pNode, data, ref);
        rebalance(pNode);
    }

    void internal_copy(Node*& dst, Node* src) override {
        if(!src) { dst = nullptr; return; }
        dst = new Node(src->m_data, src->m_ref);
        dst->m_height = src->m_height;
        internal_copy(dst->m_pChild[0], src->m_pChild[0]);
        internal_copy(dst->m_pChild[1], src->m_pChild[1]);
    }

public:
    size_t    height()  { return this->m_pRoot ? this->m_pRoot->m_height : 0; }
    ptrdiff_t balance() { return this->m_pRoot ? balance_factor(this->m_pRoot) : 0; }
};

#endif // __AVL_H__
