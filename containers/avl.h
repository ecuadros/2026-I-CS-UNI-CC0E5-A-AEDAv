#ifndef __AVL_H__
#define __AVL_H__

#include <algorithm>
#include <cstddef>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <utility>
#include "BinaryTree.h"

template<typename T, typename DerivedNode = void>
struct AVLNode : public BinaryTreeNode<
    T,
    std::conditional_t<std::is_void_v<DerivedNode>, AVLNode<T, DerivedNode>, DerivedNode>
> {
    using Node = std::conditional_t<std::is_void_v<DerivedNode>, AVLNode<T, DerivedNode>, DerivedNode>;
    using Base = BinaryTreeNode<T, Node>;
    using value_type = typename Base::value_type;

    size_t m_height;

    AVLNode(T data, Ref ref) : Base(data, ref), m_height(1) {}
};

template<typename Trait>
class AVL : public BinaryTree<Trait> {
public:
    using Base = BinaryTree<Trait>;
    using value_type = typename Trait::value_type;
    using Node = typename Trait::Node;
    using MySelf = AVL<Trait>;

private:
    size_t height(Node *node) const {
        return node ? node->m_height : 0;
    }

    void update_height(Node *node) {
        if (!node) {
            return;
        }
        node->m_height = 1 + std::max(height(node->m_pChild[0]), height(node->m_pChild[1]));
    }

    int balance_factor(Node *node) const {
        if (!node) {
            return 0;
        }
        return static_cast<int>(height(node->m_pChild[0]))
             - static_cast<int>(height(node->m_pChild[1]));
    }

    void rotate_right(Node* &node) {
        Node *newRoot = node->m_pChild[0];
        Node *moved = newRoot->m_pChild[1];

        newRoot->m_pChild[1] = node;
        node->m_pChild[0] = moved;

        update_height(node);
        update_height(newRoot);
        node = newRoot;
    }

    void rotate_left(Node* &node) {
        Node *newRoot = node->m_pChild[1];
        Node *moved = newRoot->m_pChild[0];

        newRoot->m_pChild[0] = node;
        node->m_pChild[1] = moved;

        update_height(node);
        update_height(newRoot);
        node = newRoot;
    }

    void rebalance(Node* &node) {
        if (!node) {
            return;
        }

        update_height(node);
        int factor = balance_factor(node);

        if (factor > 1) {
            if (balance_factor(node->m_pChild[0]) < 0) {
                rotate_left(node->m_pChild[0]);
            }
            rotate_right(node);
            return;
        }

        if (factor < -1) {
            if (balance_factor(node->m_pChild[1]) > 0) {
                rotate_right(node->m_pChild[1]);
            }
            rotate_left(node);
        }
    }

protected:
    void internal_insert(Node* &pNode, const value_type &data, Ref ref) override {
        if (!pNode) {
            pNode = new Node(data, ref);
            return;
        }

        auto branch = !this->m_comp(data, pNode->m_data);
        internal_insert(pNode->m_pChild[branch], data, ref);
        rebalance(pNode);
    }

    Node* internal_copy(Node *pNode) const override {
        if (!pNode) {
            return nullptr;
        }

        Node *newNode = new Node(pNode->m_data, pNode->m_ref);
        newNode->m_height = pNode->m_height;
        newNode->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        newNode->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return newNode;
    }

public:
    AVL() : Base() {}
    AVL(const MySelf &other) : Base(other) {}
    AVL(MySelf &&other) : Base(std::move(other)) {}

    MySelf& operator=(const MySelf &other) {
        Base::operator=(other);
        return *this;
    }

    MySelf& operator=(MySelf &&other) {
        Base::operator=(std::move(other));
        return *this;
    }

    ~AVL() override = default;

    size_t height() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return height(this->m_pRoot);
    }

    int balance() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return balance_factor(this->m_pRoot);
    }
};

#endif // __AVL_H__
