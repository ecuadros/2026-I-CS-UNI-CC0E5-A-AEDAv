#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <optional>
#include <tuple>
#include <shared_mutex>
#include "stack.h"
#include "../types.h"
#include "traits.h"
using namespace std;

// Nodo binario
template<typename T, typename DerivedNode = void>
struct BinaryTreeNode{
    using value_type = T;
    using Node = std::conditional_t<std::is_void_v<DerivedNode>,BinaryTreeNode, DerivedNode>;
    T m_data;
    Ref            m_ref;
    Node *m_pChild[2];
    BinaryTreeNode(T data, Ref ref): m_data(data), m_ref(ref), m_pChild{nullptr, nullptr} {}
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

// Iterador inorder perezoso (sin precopiar todos los nodos)
template<typename Node, typename value_type>
class InorderIterator {
    Stack<Node*> m_stack;
public:
    using reference = value_type&;
    InorderIterator() {}
    explicit InorderIterator(Node* root) { pushLeft(root); }
    reference operator*() const { return m_stack.top()->m_data; }
    Node* getNode() const { return m_stack.empty() ? nullptr : m_stack.top(); }
    InorderIterator& operator++() {
        if (m_stack.empty()) return *this;
        Node* node = m_stack.top();
        m_stack.pop();
        if (node->m_pChild[1]) pushLeft(node->m_pChild[1]);
        return *this;
    }
    bool operator==(const InorderIterator& o) const {
        if (m_stack.empty() && o.m_stack.empty()) return true;
        if (m_stack.empty() || o.m_stack.empty()) return false;
        return m_stack.top() == o.m_stack.top();
    }
    bool operator!=(const InorderIterator& o) const { return !(*this == o); }
private:
    void pushLeft(Node* n) {
        while (n) { m_stack.push(n); n = n->m_pChild[0]; }
    }
};

// Iterador inorder inverso perezoso
template<typename Node, typename value_type>
class InorderReverseIterator {
    Stack<Node*> m_stack;
public:
    using reference = value_type&;
    InorderReverseIterator() {}
    explicit InorderReverseIterator(Node* root) { pushRight(root); }
    reference operator*() const { return m_stack.top()->m_data; }
    Node* getNode() const { return m_stack.empty() ? nullptr : m_stack.top(); }
    InorderReverseIterator& operator++() {
        if (m_stack.empty()) return *this;
        Node* node = m_stack.top();
        m_stack.pop();
        if (node->m_pChild[0]) pushRight(node->m_pChild[0]);
        return *this;
    }
    bool operator==(const InorderReverseIterator& o) const {
        if (m_stack.empty() && o.m_stack.empty()) return true;
        if (m_stack.empty() || o.m_stack.empty()) return false;
        return m_stack.top() == o.m_stack.top();
    }
    bool operator!=(const InorderReverseIterator& o) const { return !(*this == o); }
private:
    void pushRight(Node* n) {
        while (n) { m_stack.push(n); n = n->m_pChild[1]; }
    }
};

// Iterador preorder perezoso
template<typename Node, typename value_type>
class PreorderIterator {
    Stack<Node*> m_stack;
public:
    using reference = value_type&;
    PreorderIterator() {}
    explicit PreorderIterator(Node* root) { if (root) m_stack.push(root); }
    reference operator*() const { return m_stack.top()->m_data; }
    Node* getNode() const { return m_stack.empty() ? nullptr : m_stack.top(); }
    PreorderIterator& operator++() {
        if (m_stack.empty()) return *this;
        Node* node = m_stack.top();
        m_stack.pop();
        if (node->m_pChild[1]) m_stack.push(node->m_pChild[1]);
        if (node->m_pChild[0]) m_stack.push(node->m_pChild[0]);
        return *this;
    }
    bool operator==(const PreorderIterator& o) const {
        if (m_stack.empty() && o.m_stack.empty()) return true;
        if (m_stack.empty() || o.m_stack.empty()) return false;
        return m_stack.top() == o.m_stack.top();
    }
    bool operator!=(const PreorderIterator& o) const { return !(*this == o); }
};

// Iterador postorder perezoso
template<typename Node, typename value_type>
class PostorderIterator {
    Stack<Node*> m_stack;
    Node*        m_last = nullptr;
public:
    using reference = value_type&;
    PostorderIterator() {}
    explicit PostorderIterator(Node* root) { pushLeft(root); }
    reference operator*() const { return m_stack.top()->m_data; }
    Node* getNode() const { return m_stack.empty() ? nullptr : m_stack.top(); }
    PostorderIterator& operator++() {
        if (m_stack.empty()) return *this;
        Node* top = m_stack.top();
        if (top->m_pChild[1] && m_last != top->m_pChild[1]) {
            pushLeft(top->m_pChild[1]);
        } else {
            m_stack.pop();
            m_last = top;
        }
        return *this;
    }
    bool operator==(const PostorderIterator& o) const {
        if (m_stack.empty() && o.m_stack.empty()) return true;
        if (m_stack.empty() || o.m_stack.empty()) return false;
        return m_stack.top() == o.m_stack.top();
    }
    bool operator!=(const PostorderIterator& o) const { return !(*this == o); }
private:
    void pushLeft(Node* n) {
        while (n) { m_stack.push(n); n = n->m_pChild[0]; }
    }
};

//t18 TraversalRange 
template<typename ForwardIt, typename BackwardIt>
class TraversalRange {
    ForwardIt  m_begin;
    ForwardIt  m_end;
    BackwardIt m_rbegin;
    BackwardIt m_rend;
public:
    TraversalRange(ForwardIt b, ForwardIt e, BackwardIt rb, BackwardIt re): m_begin(b), m_end(e), m_rbegin(rb), m_rend(re) {}
    ForwardIt  begin()  const { return m_begin;  }
    ForwardIt  end()    const { return m_end;    }
    BackwardIt rbegin() const { return m_rbegin; }
    BackwardIt rend()   const { return m_rend;   }

    //t8 forEach
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
};

//binary Tree
template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    //iteradores publicos
    using ForwardIt  = BTForwardIterator <Node, value_type>;
    using BackwardIt = BTBackwardIterator<Node, value_type>;
    // iteradores inorder perezosos (iteración por defecto)
    using InorderIt     = InorderIterator<Node, value_type>;
    using InorderRevIt  = InorderReverseIterator<Node, value_type>;
    using PreorderIt    = PreorderIterator<Node, value_type>;
    using PostorderIt   = PostorderIterator<Node, value_type>;
    //t18 vistas de recorrido (usando iteradores perezosos para adelante)
    using InorderView   = TraversalRange<InorderIt, InorderRevIt>;
    using PreorderView  = TraversalRange<PreorderIt, BackwardIt>;
    using PostorderView = TraversalRange<PostorderIt, BackwardIt>;

protected:
    Node                *m_pRoot;
    Comp                 m_comp;
    mutable shared_mutex m_mtx;

    //inserta nodo recursivo
    virtual void internal_insert(Node* &pNode, const value_type &data, Ref ref){
        if(!pNode){pNode = new Node(data, ref); return; }
        auto branch=!m_comp(data, pNode->m_data);
        internal_insert(pNode->m_pChild[branch], data, ref);
    }

    //t5 elimina nodo con hijos recursivo
    virtual void internal_clear(Node* pNode) {
        if(!pNode)return;
        internal_clear(pNode->m_pChild[0]);
        internal_clear(pNode->m_pChild[1]);
        delete pNode;
    }
    //t3 constructor de copia del nodo recursivo
    virtual Node* internal_copy(Node* pNode) {
        if (!pNode) return nullptr;
        Node* n = new Node(pNode->m_data, pNode->m_ref);
        n->m_pChild[0]=internal_copy(pNode->m_pChild[0]);
        n->m_pChild[1]=internal_copy(pNode->m_pChild[1]);
        return n;
    }
    //t17 búsqueda recursiva del nodo
    virtual Node* internal_search(Node* pNode, const value_type& data) const {
        if (!pNode) return nullptr;
        if (!m_comp(data, pNode->m_data) && !m_comp(pNode->m_data, data))
            return pNode; 
        auto branch = !m_comp(data, pNode->m_data);
        return internal_search(pNode->m_pChild[branch], data);
    }
    //size recursivo
    virtual size_t internal_size(Node* n) const {
        if (!n) return 0;
        return 1 + internal_size(n->m_pChild[0]) + internal_size(n->m_pChild[1]);
    }

    //t6 t7 inorder 
    void fill_inorder(Node* n, Stack<Node*>& s) const {
        if (!n) return;
        fill_inorder(n->m_pChild[0], s);
        s.push(n);
        fill_inorder(n->m_pChild[1], s);
    }
    //t13 t14 preorder 
    void fill_preorder(Node* n, Stack<Node*>& s) const {
        if (!n) return;
        s.push(n);
        fill_preorder(n->m_pChild[0], s);
        fill_preorder(n->m_pChild[1], s);
    }
    //t15 t16 postorder
    void fill_postorder(Node* n, Stack<Node*>& s) const {
        if (!n) return;
        fill_postorder(n->m_pChild[0], s);
        fill_postorder(n->m_pChild[1], s);
        s.push(n);
    }

    // Las vistas usan iteradores perezosos en forward y una pila copiada en reverse.

    // Convertir recorridos a texto desde la pila
    string traversalToString(Stack<Node*>& s) const {
        ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < s.size(); ++i) {
            if (i) oss << ",";
            oss<< "("<< s[i]->m_data<< ","<< s[i]->m_ref<< ")";
        }
        oss<< "]";
        return oss.str();
    }

public:
    BinaryTree() : m_pRoot(nullptr) {}

    //t3 constructor de copia
    BinaryTree(const BinaryTree& other) : m_pRoot(nullptr) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = internal_copy(other.m_pRoot);
    }

    //t4 constructor de movimiento
    BinaryTree(BinaryTree&& other) : m_pRoot(nullptr) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = exchange(other.m_pRoot, nullptr);
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
            m_pRoot = exchange(other.m_pRoot, nullptr);
        }
        return *this;
    }

    //t5 destructor seguro
    virtual ~BinaryTree() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_clear(m_pRoot);
        m_pRoot = nullptr;
    }

    //insert
    void insert(const value_type& data, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, data, ref);
    }


    //t17 contains: pregunta si un valor está dentro del árbol
    bool contains(const value_type& data) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_search(m_pRoot, data) != nullptr;
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return internal_size(m_pRoot);
    }

    //t6 t7 vista inorder
    InorderView inorder() const {
            shared_lock<shared_mutex> lock(m_mtx);
            // adelante: iterador inorder perezoso; atrás: iterador inverso perezoso
            return InorderView(InorderIt(m_pRoot), InorderIt(), InorderRevIt(m_pRoot), InorderRevIt());
    }
    
    //t13 t14 vista preorder
    PreorderView preorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        // adelante: iterador preorder perezoso
        // atrás: copia de pila para el recorrido inverso
        Stack<Node*> s;
        fill_preorder(m_pRoot, s);
        ptrdiff_t last = (ptrdiff_t)s.size() - 1;
        return PreorderView(PreorderIt(m_pRoot), PreorderIt(), BackwardIt(s, last), BackwardIt(s, -1));
    }

    //t15 t16 vista postorder
    PostorderView postorder() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s;
        fill_postorder(m_pRoot, s);
        ptrdiff_t last = (ptrdiff_t)s.size() - 1;
        return PostorderView(PostorderIt(m_pRoot), PostorderIt(), BackwardIt(s, last), BackwardIt(s, -1));
    }

    // begin/end con iteradores inorder perezosos
    InorderIt begin() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* root = m_pRoot;
        return InorderIt(root);
    }
    InorderIt end() const { return InorderIt(); }

    // recorrido inorder inverso
    InorderRevIt rbegin() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* root = m_pRoot;
        return InorderRevIt(root);
    }
    InorderRevIt rend() const { return InorderRevIt(); }

    //t9 toString
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        Stack<Node*> s;
        string result;
        fill_inorder(m_pRoot, s);
        result += "Inorder:" + traversalToString(s) + "\n";
        while (!s.empty()) s.pop();
        fill_preorder(m_pRoot, s);
        result += "Preorder:" + traversalToString(s) + "\n";
        while (!s.empty()) s.pop();
        fill_postorder(m_pRoot, s);
        result += "Postorder:" + traversalToString(s);
        return result;
    }

    //t10 operator<< consola y archivo
    friend ostream& operator<<(ostream& os, const BinaryTree& tree) {
        shared_lock<shared_mutex> lock(tree.m_mtx);
        Stack<Node*> s;
        tree.fill_inorder(tree.m_pRoot, s);
        os<< tree.traversalToString(s);
        return os;
    }

    //t11 operator>>
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

    // Imprimir árbol
    void printTree(ostream& os = cout) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) { os<< "(arbol vacio)"<< endl; return; }
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
            os<< "  ";
            for (size_t i = 0; i < level_size; ++i) {
                Node* n = queue[front++];
                if (n) {
                    os<< n->m_data;
                    queue.push_back(n->m_pChild[0], 0);
                    queue.push_back(n->m_pChild[1], 0);
                } else {
                    os<< "_";
                    queue.push_back(nullptr, 0);
                    queue.push_back(nullptr, 0);
                }
                os<< " ";
            }
            os<< endl;
        }
    }
};

#endif // __BINARYTREE_H__