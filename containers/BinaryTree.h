#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <cstddef>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include "../types.h"
#include "traits.h"
using namespace std;

template<typename T, typename DerivedNode = void>
struct BinaryTreeNode {
    using value_type = T;
    using Node = std::conditional_t<
        std::is_void_v<DerivedNode>,
        BinaryTreeNode<T, DerivedNode>,
        DerivedNode
    >;

    T     m_data;
    Ref   m_ref;
    Node *m_pChild[2];

    BinaryTreeNode(T data, Ref ref)
        : m_data(data), m_ref(ref), m_pChild{nullptr, nullptr} {}
};

// Utilizar:
//    BinaryTree<AscendingTrait<BinaryTreeNode<T>>>
//    BinaryTree<DescendingTrait<BinaryTreeNode<T>>>
template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = BinaryTree<Trait>;

protected:
    Node                *m_pRoot = nullptr;
    Comp                 m_comp;
    mutable shared_mutex m_mtx;

    virtual void internal_insert(Node* &pNode, const value_type &data, Ref ref) {
        if (!pNode) {
            pNode = new Node(data, ref);
            return;
        }

        auto branch = !m_comp(data, pNode->m_data);
        internal_insert(pNode->m_pChild[branch], data, ref);
    }

    virtual void internal_clear(Node *pNode) {
        if (!pNode) {
            return;
        }

        internal_clear(pNode->m_pChild[0]);
        internal_clear(pNode->m_pChild[1]);
        delete pNode;
    }

    virtual Node* internal_copy(Node *pNode) const {
        if (!pNode) {
            return nullptr;
        }

        Node *newNode = new Node(pNode->m_data, pNode->m_ref);
        newNode->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        newNode->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return newNode;
    }

    virtual Node* internal_search(Node *pNode, const value_type &data) const {
        if (!pNode) {
            return nullptr;
        }

        if (!m_comp(data, pNode->m_data) && !m_comp(pNode->m_data, data)) {
            return pNode;
        }

        auto branch = !m_comp(data, pNode->m_data);
        return internal_search(pNode->m_pChild[branch], data);
    }

    virtual size_t internal_size(Node *pNode) const {
        if (!pNode) {
            return 0;
        }

        return 1 + internal_size(pNode->m_pChild[0]) + internal_size(pNode->m_pChild[1]);
    }

    void append_node(ostringstream &oss, Node *pNode, bool &first) const {
        if (!first) {
            oss << ",";
        }
        oss << "(" << pNode->m_data << "," << pNode->m_ref << ")";
        first = false;
    }

    void append_inorder(Node *pNode, ostringstream &oss, bool &first) const {
        if (!pNode) {
            return;
        }

        append_inorder(pNode->m_pChild[0], oss, first);
        append_node(oss, pNode, first);
        append_inorder(pNode->m_pChild[1], oss, first);
    }

    void append_preorder(Node *pNode, ostringstream &oss, bool &first) const {
        if (!pNode) {
            return;
        }

        append_node(oss, pNode, first);
        append_preorder(pNode->m_pChild[0], oss, first);
        append_preorder(pNode->m_pChild[1], oss, first);
    }

    void append_postorder(Node *pNode, ostringstream &oss, bool &first) const {
        if (!pNode) {
            return;
        }

        append_postorder(pNode->m_pChild[0], oss, first);
        append_postorder(pNode->m_pChild[1], oss, first);
        append_node(oss, pNode, first);
    }

    string internal_inorder_string() const {
        ostringstream oss;
        bool first = true;
        oss << "[";
        append_inorder(m_pRoot, oss, first);
        oss << "]";
        return oss.str();
    }

    string internal_preorder_string() const {
        ostringstream oss;
        bool first = true;
        oss << "[";
        append_preorder(m_pRoot, oss, first);
        oss << "]";
        return oss.str();
    }

    string internal_postorder_string() const {
        ostringstream oss;
        bool first = true;
        oss << "[";
        append_postorder(m_pRoot, oss, first);
        oss << "]";
        return oss.str();
    }

    void internal_print_tree(Node *pNode, ostream &os, size_t depth) const {
        if (!pNode) {
            return;
        }

        internal_print_tree(pNode->m_pChild[1], os, depth + 1);
        for (size_t i = 0; i < depth; ++i) {
            os << "    ";
        }
        os << "(" << pNode->m_data << "," << pNode->m_ref << ")" << endl;
        internal_print_tree(pNode->m_pChild[0], os, depth + 1);
    }

public:
    BinaryTree() = default;

    BinaryTree(const MySelf &other) : m_pRoot(nullptr), m_comp(other.m_comp) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = internal_copy(other.m_pRoot);
    }

    BinaryTree(MySelf &&other) : m_pRoot(nullptr), m_comp(std::move(other.m_comp)) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
    }

    MySelf& operator=(const MySelf &other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            m_comp = other.m_comp;
            m_pRoot = internal_copy(other.m_pRoot);
        }
        return *this;
    }

    MySelf& operator=(MySelf &&other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_comp = std::move(other.m_comp);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
        }
        return *this;
    }

    virtual ~BinaryTree() {
        clear();
    }

    virtual void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_clear(m_pRoot);
        m_pRoot = nullptr;
    }

    virtual void insert(const value_type &data, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, data, ref);
    }

    tuple<value_type, Ref> search(const value_type &data) const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node *found = internal_search(m_pRoot, data);
        if (!found) {
            throw runtime_error("elemento no encontrado");
        }
        return make_tuple(found->m_data, found->m_ref);
    }

    bool contains(const value_type &data) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_search(m_pRoot, data) != nullptr;
    }

    bool isEmpty() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_pRoot == nullptr;
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_size(m_pRoot);
    }

    string inorderToString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_inorder_string();
    }

    string preorderToString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_preorder_string();
    }

    string postorderToString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_postorder_string();
    }

    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return "Inorder:   " + internal_inorder_string() + "\n"
             + "Preorder:  " + internal_preorder_string() + "\n"
             + "Postorder: " + internal_postorder_string();
    }

    void printTree(ostream &os = cout) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) {
            os << "(arbol vacio)" << endl;
            return;
        }
        internal_print_tree(m_pRoot, os, 0);
    }

    friend ostream& operator<<(ostream &os, const MySelf &tree) {
        shared_lock<shared_mutex> lock(tree.m_mtx);
        return os << tree.internal_inorder_string();
    }

    friend istream& operator>>(istream &is, MySelf &tree) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return is;
        }

        value_type val;
        Ref ref;
        char comma;
        char closeParen;

        while (is >> ch && ch != ']') {
            if (ch == '(' && is >> val >> comma >> ref >> closeParen) {
                if (comma == ',' && closeParen == ')') {
                    tree.insert(val, ref);
                }
            }
        }
        return is;
    }
};

#endif // __BINARYTREE_H__
