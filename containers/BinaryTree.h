#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <shared_mutex>
#include "stack.h"
#include "vector.h"
#include "../types.h"
#include "traits.h"
using namespace std;

template<typename T, typename DerivedNode = void>
struct BinaryTreeNode {
    using value_type = T;
    using Node = std::conditional_t<std::is_void_v<DerivedNode>,
                                    BinaryTreeNode, DerivedNode>;
    T     m_data;
    Ref   m_ref;
    Node *m_pChild[2];
    BinaryTreeNode(T data, Ref ref)
        : m_data(data), m_ref(ref), m_pChild{nullptr, nullptr} {}
};

template<typename Node, typename value_type>
class BTIteratorBase {
protected:
    Stack<Node*> m_nodes;
    ptrdiff_t    m_index;
public:
    BTIteratorBase() : m_index(0) {}
    BTIteratorBase(Stack<Node*> s, ptrdiff_t idx) : m_nodes(s), m_index(idx) {}
    value_type& operator*()  const { return m_nodes[m_index]->m_data; }
    Node*       getNode()    const { return m_nodes[m_index]; }
    bool operator==(const BTIteratorBase& o) const { return m_index == o.m_index; }
    bool operator!=(const BTIteratorBase& o) const { return m_index != o.m_index; }
};

template<typename Node, typename value_type>
class BTForwardIterator : public BTIteratorBase<Node, value_type> {
public:
    using BTIteratorBase<Node, value_type>::BTIteratorBase;
    BTForwardIterator& operator++() { ++this->m_index; return *this; }
};

template<typename Node, typename value_type>
class BTBackwardIterator : public BTIteratorBase<Node, value_type> {
public:
    using BTIteratorBase<Node, value_type>::BTIteratorBase;
    BTBackwardIterator& operator++() { --this->m_index; return *this; }
};

template<typename ForwardIt, typename BackwardIt>
class TraversalView {
    ForwardIt  m_begin;
    ForwardIt  m_end;
    BackwardIt m_rbegin;
    BackwardIt m_rend;
public:
    TraversalView(ForwardIt b, ForwardIt e, BackwardIt rb, BackwardIt re)
        : m_begin(b), m_end(e), m_rbegin(rb), m_rend(re) {}
    ForwardIt  begin()  const { return m_begin;  }
    ForwardIt  end()    const { return m_end;    }
    BackwardIt rbegin() const { return m_rbegin; }
    BackwardIt rend()   const { return m_rend;   }

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
};

template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using ForwardIt  = BTForwardIterator <Node, value_type>;
    using BackwardIt = BTBackwardIterator<Node, value_type>;
    using InorderView   = TraversalView<ForwardIt, BackwardIt>;
    using PreorderView  = TraversalView<ForwardIt, BackwardIt>;
    using PostorderView = TraversalView<ForwardIt, BackwardIt>;

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
        Node* n = new Node(pNode->m_data, pNode->m_ref);
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

    TraversalView<ForwardIt,BackwardIt> make_view(Stack<Node*> s) const {
        ptrdiff_t last = (ptrdiff_t)s.size() - 1;
        ForwardIt  b(s, 0),    e(s, (ptrdiff_t)s.size());
        BackwardIt rb(s, last), re(s, -1);
        return {b, e, rb, re};
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

public:
    BinaryTree() : m_pRoot(nullptr) {}

    BinaryTree(const BinaryTree& other) : m_pRoot(nullptr) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = internal_copy(other.m_pRoot);
    }

    BinaryTree(BinaryTree&& other) : m_pRoot(nullptr) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot       = other.m_pRoot;
        other.m_pRoot = nullptr;
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
            m_pRoot       = other.m_pRoot;
            other.m_pRoot = nullptr;
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

    tuple<value_type, Ref> search(const value_type& data) const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* found = internal_search(m_pRoot, data);
        if (!found) throw runtime_error("elemento no encontrado");
        return make_tuple(found->m_data, found->m_ref);
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_size(m_pRoot);
    }
    InorderView inorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s; fill_inorder(m_pRoot, s);
        return make_view(s);
    }
    PreorderView preorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s; fill_preorder(m_pRoot, s);
        return make_view(s);
    }
    PostorderView postorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s; fill_postorder(m_pRoot, s);
        return make_view(s);
    }
    ForwardIt begin() const { return inorder().begin(); }
    ForwardIt end()   const { return inorder().end();   }

    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s;
        string result;

        fill_inorder(m_pRoot, s);
        result += "Inorder:   " + traversalToString(s) + "\n";
        while (!s.empty()) s.pop();

        fill_preorder(m_pRoot, s);
        result += "Preorder:  " + traversalToString(s) + "\n";
        while (!s.empty()) s.pop();

        fill_postorder(m_pRoot, s);
        result += "Postorder: " + traversalToString(s);

        return result;
    }

    void printTree(ostream& os = cout) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) { os << "  (arbol vacio)" << endl; return; }

        Vector<Node*> queue(64);
        size_t front = 0;
        queue.push_back(m_pRoot, 0);

        while (front < queue.size()) {
            size_t level_size = queue.size() - front;
            bool   all_null   = true;

            for (size_t i = 0; i < level_size; ++i) {
                Node* n = queue[front + i];
                if (n) { all_null = false; break; }
            }
            if (all_null) break;

            os << "  ";
            for (size_t i = 0; i < level_size; ++i) {
                Node* n = queue[front++];
                if (n) {
                    os << n->m_data;
                    queue.push_back(n->m_pChild[0], 0);
                    queue.push_back(n->m_pChild[1], 0);
                } else {
                    os << "_";
                    queue.push_back(nullptr, 0);
                    queue.push_back(nullptr, 0);
                }
                os << " ";
            }
            os << endl;
        }
    }


    friend ostream& operator<<(ostream& os, const BinaryTree& tree) {
        shared_lock<shared_mutex> lock(tree.m_mtx);
        Stack<Node*> s;
        tree.fill_inorder(tree.m_pRoot, s);
        os << tree.traversalToString(s);
        return os;
    }

    friend istream& operator>>(istream& is, BinaryTree& tree) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type val; Ref ref; char comma, paren;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> val >> comma >> ref >> paren)
                    if (comma == ',' && paren == ')')
                        tree.insert(val, ref);
        return is;
    }
};

#endif // __BINARYTREE_H__