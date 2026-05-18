#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

// Error #3
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "types.h"
#include "containers/traits.h"
#include "containers/general_iterator.h"
#include "containers/util.h"

template <typename T>
struct BinaryTreeNode{
    using value_type = T;
    using Node = BinaryTreeNode<T>;

    T m_data;
    Ref m_ref;
    Node *m_pChild[2];

    BinaryTreeNode()
        : m_data(T()), m_ref(Ref()), m_pChild{nullptr, nullptr} {}

    BinaryTreeNode(const T &data, Ref ref = Ref())
        : m_data(data), m_ref(ref), m_pChild{nullptr, nullptr} {}

    T getData() const { return m_data; }
    T &getDataRef() { return m_data; }
    const T &getDataRef() const { return m_data; }
    void setData(const T &data) { m_data = data; }
    Ref getRef() const { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
};

template <typename T>
bool operator<(const BinaryTreeNode<T> &lhs, const BinaryTreeNode<T> &rhs){
    return lhs.getData() < rhs.getData();
}

template <typename T>
bool operator<=(const BinaryTreeNode<T> &lhs, const BinaryTreeNode<T> &rhs){
    return !(rhs < lhs);
}

template <typename T>
bool operator>(const BinaryTreeNode<T> &lhs, const BinaryTreeNode<T> &rhs){
    return rhs < lhs;
}

template <typename T>
bool operator>=(const BinaryTreeNode<T> &lhs, const BinaryTreeNode<T> &rhs){
    return !(lhs < rhs);
}

template <typename T>
using AscendingBinaryTreeTrait = AscendingTrait<BinaryTreeNode<T>>;

template <typename T>
using DescendingBinaryTreeTrait = DescendingTrait<BinaryTreeNode<T>>;

enum class TreeTraversal{
    inorder,
    preorder,
    postorder
};

template <typename Container, TreeTraversal Traversal, bool Reverse>
class BinaryTreeIterator : public general_iterator<Container, BinaryTreeIterator<Container, Traversal, Reverse>>{
public:
    using MySelf = BinaryTreeIterator<Container, Traversal, Reverse>;
    using Parent = general_iterator<Container, MySelf>;
    using Node = typename Container::Node;

private:
    std::shared_ptr<std::vector<Node *>> m_snapshot;
    std::size_t m_index;

    static Node *current_node(const std::shared_ptr<std::vector<Node *>> &snapshot, std::size_t index){
        if(!snapshot || index >= snapshot->size()){
            return nullptr;
        }
        return (*snapshot)[index];
    }

public:
    BinaryTreeIterator(
        Container *pContainer = nullptr,
        std::shared_ptr<std::vector<Node *>> snapshot = std::make_shared<std::vector<Node *>>(),
        std::size_t index = 0)
        : Parent(pContainer, current_node(snapshot, index)),
          m_snapshot(std::move(snapshot)),
          m_index(index) {}

    MySelf operator++(){
        if(m_snapshot && m_index < m_snapshot->size()){
            ++m_index;
        }
        this->m_pNode = current_node(m_snapshot, m_index);
        return *this;
    }
};

template <typename Trait>
class BinaryTree{
public:
    using value_type = typename Trait::value_type;
    using Node = typename Trait::Node;

    // Error #2
    using Comp = typename Trait::Comp;
    using Compare = typename Trait::Compare;
    using MySelf = BinaryTree<Trait>;

    using forward_iterator = BinaryTreeIterator<MySelf, TreeTraversal::inorder, false>;
    friend forward_iterator;
    using backward_iterator = BinaryTreeIterator<MySelf, TreeTraversal::inorder, true>;
    friend backward_iterator;
    using preorder_forward_iterator = BinaryTreeIterator<MySelf, TreeTraversal::preorder, false>;
    friend preorder_forward_iterator;
    using preorder_backward_iterator = BinaryTreeIterator<MySelf, TreeTraversal::preorder, true>;
    friend preorder_backward_iterator;
    using postorder_forward_iterator = BinaryTreeIterator<MySelf, TreeTraversal::postorder, false>;
    friend postorder_forward_iterator;
    using postorder_backward_iterator = BinaryTreeIterator<MySelf, TreeTraversal::postorder, true>;
    friend postorder_backward_iterator;

protected:
    Node *m_pRoot;
    std::size_t m_size;
    Comp m_comp;

    // Concurency
    mutable std::shared_mutex m_mtx;

    // Utilizacion de if
    bool is_equal(const value_type &lhs, const value_type &rhs) const{
        return !m_comp(lhs, rhs) && !m_comp(rhs, lhs);
    }

    std::size_t child_index(const value_type &data, const Node *node) const{
        return m_comp(data, node->m_data) ? 0u : 1u;
    }

    template <typename NodeType>
    static int node_height(const NodeType *node){
        if(node == nullptr){
            return 0;
        }
        if constexpr (requires(const NodeType *item) { item->m_height; }){
            return node->m_height;
        }
        return 1;
    }

    template <typename NodeType>
    static void refresh_node(NodeType *node){
        if(node == nullptr){
            return;
        }
        if constexpr (requires(NodeType *item) { item->m_height; }){
            node->m_height = 1 + std::max(
                node_height(node->m_pChild[0]),
                node_height(node->m_pChild[1]));
        }
    }

    static void destroy_nodes(Node *node){
        if(node == nullptr){
            return;
        }
        destroy_nodes(node->m_pChild[0]);
        destroy_nodes(node->m_pChild[1]);
        delete node;
    }

    static Node *clone_nodes(const Node *node){
        if(node == nullptr){
            return nullptr;
        }
        Node *copy = new Node(*node);
        copy->m_pChild[0] = nullptr;
        copy->m_pChild[1] = nullptr;
        try{
            copy->m_pChild[0] = clone_nodes(node->m_pChild[0]);
            copy->m_pChild[1] = clone_nodes(node->m_pChild[1]);
        }catch(...){
            destroy_nodes(copy->m_pChild[0]);
            destroy_nodes(copy->m_pChild[1]);
            delete copy;
            throw;
        }
        return copy;
    }

    static std::size_t count_nodes(const Node *node){
        if(node == nullptr){
            return 0;
        }
        return 1 + count_nodes(node->m_pChild[0]) + count_nodes(node->m_pChild[1]);
    }

    void clear_unlocked(){
        destroy_nodes(m_pRoot);
        m_pRoot = nullptr;
        m_size = 0;
    }

    template <typename Fixup>
    void internal_insert_impl(Node *&pNode, const value_type &data, Ref ref, Fixup fixup){
        if(pNode == nullptr){
            pNode = new Node(data, ref);
            ++m_size;
            return;
        }
        const std::size_t branch = child_index(data, pNode);
        internal_insert_impl(pNode->m_pChild[branch], data, ref, fixup);
        fixup(pNode);
    }

    void internal_insert(Node *&pNode, const value_type &data, Ref ref){
        internal_insert_impl(pNode, data, ref, [](Node *&) {});
    }

    void collect_nodes(Node *node, std::vector<Node *> &nodes, TreeTraversal traversal) const{
        if(node == nullptr){
            return;
        }
        if(traversal == TreeTraversal::preorder){
            nodes.push_back(node);
        }
        collect_nodes(node->m_pChild[0], nodes, traversal);
        if(traversal == TreeTraversal::inorder){
            nodes.push_back(node);
        }
        collect_nodes(node->m_pChild[1], nodes, traversal);
        if(traversal == TreeTraversal::postorder){
            nodes.push_back(node);
        }
    }

    std::shared_ptr<std::vector<Node *>> make_snapshot(TreeTraversal traversal, bool reverse = false) const{
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        auto nodes = std::make_shared<std::vector<Node *>>();
        nodes->reserve(m_size);
        collect_nodes(m_pRoot, *nodes, traversal);
        if(reverse){
            std::reverse(nodes->begin(), nodes->end());
        }
        return nodes;
    }

    void print_node(std::ostream &os, Node *node, bool &first) const{
        if(node == nullptr){
            return;
        }
        print_node(os, node->m_pChild[0], first);
        if(!first){
            os << " ";
        }
        os << node->getData();
        first = false;
        print_node(os, node->m_pChild[1], first);
    }

    Node *find_node_unlocked(const value_type &data) const{
        Node *current = m_pRoot;
        while(current != nullptr){
            if(m_comp(data, current->m_data)){
                current = current->m_pChild[0];
            }else if(m_comp(current->m_data, data)){
                current = current->m_pChild[1];
            }else{
                return current;
            }
        }
        return nullptr;
    }

    template <TreeTraversal Traversal, bool Reverse>
    BinaryTreeIterator<MySelf, Traversal, Reverse> make_iterator(){
        return BinaryTreeIterator<MySelf, Traversal, Reverse>(this, make_snapshot(Traversal, Reverse), 0);
    }

    template <TreeTraversal Traversal, bool Reverse>
    BinaryTreeIterator<MySelf, Traversal, Reverse> make_iterator() const{
        return BinaryTreeIterator<MySelf, Traversal, Reverse>(
            const_cast<MySelf *>(this),
            make_snapshot(Traversal, Reverse),
            0);
    }

    void write_node(std::ostream &os, const Node *node) const{
        if(node == nullptr){
            os << 0 << '\n';
            return;
        }
        os << 1 << ' ' << node->getData() << ' ' << node->getRef() << '\n';
        write_node(os, node->m_pChild[0]);
        write_node(os, node->m_pChild[1]);
    }

    Node *read_node(std::istream &is){
        int flag = 0;
        if(!(is >> flag)){
            throw std::runtime_error("Formato invalido");
        }
        if(flag == 0){
            return nullptr;
        }
        value_type data{};
        Ref ref{};
        if(!(is >> data >> ref)){
            throw std::runtime_error("Formato invalido");
        }
        std::unique_ptr<Node> node(new Node(data, ref));
        node->m_pChild[0] = nullptr;
        node->m_pChild[1] = nullptr;
        try{
            node->m_pChild[0] = read_node(is);
            node->m_pChild[1] = read_node(is);
        }catch(...){
            destroy_nodes(node->m_pChild[0]);
            destroy_nodes(node->m_pChild[1]);
            throw;
        }
        refresh_node(node.get());
        return node.release();
    }

public:
    BinaryTree()
        : m_pRoot(nullptr), m_size(0), m_comp(Comp()) {}

    // Constructor copia
    BinaryTree(const BinaryTree &other)
        : m_pRoot(nullptr), m_size(0), m_comp(other.m_comp){
        std::shared_lock<std::shared_mutex> lock(other.m_mtx);
        m_pRoot = clone_nodes(other.m_pRoot);
        m_size = other.m_size;
    }

    // Constructor move
    BinaryTree(BinaryTree &&other)
        : m_pRoot(nullptr), m_size(0), m_comp(Comp()){
        std::unique_lock<std::shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_size = std::exchange(other.m_size, 0);
        m_comp = std::move(other.m_comp);
    }

    BinaryTree &operator=(const BinaryTree &other){
        if(this != &other){
            Node *new_root = nullptr;
            std::size_t new_size = 0;
            Comp new_comp = Comp();
            {
                std::shared_lock<std::shared_mutex> lock(other.m_mtx);
                new_root = clone_nodes(other.m_pRoot);
                new_size = other.m_size;
                new_comp = other.m_comp;
            }
            std::unique_lock<std::shared_mutex> lock(m_mtx);
            clear_unlocked();
            m_pRoot = new_root;
            m_size = new_size;
            m_comp = std::move(new_comp);
        }
        return *this;
    }

    BinaryTree &operator=(BinaryTree &&other){
        if(this != &other){
            std::unique_lock<std::shared_mutex> lock_this(m_mtx, std::defer_lock);
            std::unique_lock<std::shared_mutex> lock_other(other.m_mtx, std::defer_lock);
            std::lock(lock_this, lock_other);
            clear_unlocked();
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_size = std::exchange(other.m_size, 0);
            m_comp = std::move(other.m_comp);
        }
        return *this;
    }

    // Destructor Seguro
    virtual ~BinaryTree(){
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        clear_unlocked();
    }

    // Error #1
    void insert(const value_type &data){
        insert(data, Ref());
    }

    virtual void insert(const value_type &data, Ref ref){
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, data, ref);
    }

    void clear(){
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        clear_unlocked();
    }

    std::size_t size() const{
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return m_size;
    }

    bool empty() const{
        return size() == 0;
    }

    void print(std::ostream &os = std::cout) const{
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        bool first = true;
        print_node(os, m_pRoot, first);
    }

    // usar un bucle nativo
    forward_iterator begin(){
        return make_iterator<TreeTraversal::inorder, false>();
    }

    forward_iterator end(){
        return forward_iterator(this);
    }

    forward_iterator begin() const{
        return make_iterator<TreeTraversal::inorder, false>();
    }

    forward_iterator end() const{
        return forward_iterator(const_cast<MySelf *>(this));
    }

    // ForEach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args){
        ::ForEach(begin(), end(), func, std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args) const{
        ::ForEach(begin(), end(), func, std::forward<Args>(args)...);
    }

    // forward iterator (inorder)
    forward_iterator inorder_begin(){
        return begin();
    }

    forward_iterator inorder_end(){
        return end();
    }

    forward_iterator inorder_begin() const{
        return begin();
    }

    forward_iterator inorder_end() const{
        return end();
    }

    // backward iterator (inorder)
    backward_iterator rbegin(){
        return make_iterator<TreeTraversal::inorder, true>();
    }

    backward_iterator rend(){
        return backward_iterator(this);
    }

    backward_iterator rbegin() const{
        return make_iterator<TreeTraversal::inorder, true>();
    }

    backward_iterator rend() const{
        return backward_iterator(const_cast<MySelf *>(this));
    }

    // forward iterator (preorder)
    preorder_forward_iterator preorder_begin(){
        return make_iterator<TreeTraversal::preorder, false>();
    }

    preorder_forward_iterator preorder_end(){
        return preorder_forward_iterator(this);
    }

    preorder_forward_iterator preorder_begin() const{
        return make_iterator<TreeTraversal::preorder, false>();
    }

    preorder_forward_iterator preorder_end() const{
        return preorder_forward_iterator(const_cast<MySelf *>(this));
    }

    // backward iterator (preorder)
    preorder_backward_iterator preorder_rbegin(){
        return make_iterator<TreeTraversal::preorder, true>();
    }

    preorder_backward_iterator preorder_rend(){
        return preorder_backward_iterator(this);
    }

    preorder_backward_iterator preorder_rbegin() const{
        return make_iterator<TreeTraversal::preorder, true>();
    }

    preorder_backward_iterator preorder_rend() const{
        return preorder_backward_iterator(const_cast<MySelf *>(this));
    }

    // forward iterator (postorder)
    postorder_forward_iterator postorder_begin(){
        return make_iterator<TreeTraversal::postorder, false>();
    }

    postorder_forward_iterator postorder_end(){
        return postorder_forward_iterator(this);
    }

    postorder_forward_iterator postorder_begin() const{
        return make_iterator<TreeTraversal::postorder, false>();
    }

    postorder_forward_iterator postorder_end() const{
        return postorder_forward_iterator(const_cast<MySelf *>(this));
    }

    // backward iterator (postorder)
    postorder_backward_iterator postorder_rbegin(){
        return make_iterator<TreeTraversal::postorder, true>();
    }

    postorder_backward_iterator postorder_rend(){
        return postorder_backward_iterator(this);
    }

    postorder_backward_iterator postorder_rbegin() const{
        return make_iterator<TreeTraversal::postorder, true>();
    }

    postorder_backward_iterator postorder_rend() const{
        return postorder_backward_iterator(const_cast<MySelf *>(this));
    }

    // operator<<
    friend std::ostream &operator<<(std::ostream &os, const BinaryTree &tree){
        auto nodes = tree.make_snapshot(TreeTraversal::inorder, false);
        os << "[";
        for(std::size_t i = 0; i < nodes->size(); ++i){
            Node *node = (*nodes)[i];
            os << "(" << node->getData() << "," << node->getRef() << ")";
            if(i + 1 < nodes->size()){
                os << ",";
            }
        }
        os << "]";
        return os;
    }

    // (incluye persistencia a archivo/s)
    void save(const std::string &filename) const{
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        std::ofstream output(filename);
        if(!output){
            throw std::runtime_error("No se pudo abrir el archivo");
        }
        write_node(output, m_pRoot);
    }

    void load(const std::string &filename){
        std::ifstream input(filename);
        if(!input){
            throw std::runtime_error("No se pudo abrir el archivo");
        }
        Node *new_root = nullptr;
        try{
            new_root = read_node(input);
        }catch(...){
            destroy_nodes(new_root);
            throw;
        }
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        clear_unlocked();
        m_pRoot = new_root;
        m_size = count_nodes(m_pRoot);
        refresh_node(m_pRoot);
    }

    // mejoras libre #1
    bool contains(const value_type &data) const{
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return find_node_unlocked(data) != nullptr;
    }

    // mejora libre #2
    template <typename Func>
    void Visit(TreeTraversal traversal, Func func, bool reverse = false) const{
        auto nodes = make_snapshot(traversal, reverse);
        for(Node *node : *nodes){
            func(node->getData(), node->getRef());
        }
    }
};

// Adapter BinaryTreeNode para tener la altura
template <typename T>
struct AVLNode{
    using value_type = T;
    using Node = AVLNode<T>;

    T m_data;
    Ref m_ref;
    Node *m_pChild[2];
    int m_height;

    AVLNode()
        : m_data(T()), m_ref(Ref()), m_pChild{nullptr, nullptr}, m_height(1) {}

    AVLNode(const T &data, Ref ref = Ref())
        : m_data(data), m_ref(ref), m_pChild{nullptr, nullptr}, m_height(1) {}

    T getData() const { return m_data; }
    T &getDataRef() { return m_data; }
    const T &getDataRef() const { return m_data; }
    void setData(const T &data) { m_data = data; }
    Ref getRef() const { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
};

template <typename T>
using AscendingAVLTrait = AscendingTrait<AVLNode<T>>;

template <typename T>
using DescendingAVLTrait = DescendingTrait<AVLNode<T>>;

template <typename Trait>
class AVLTree : public BinaryTree<Trait>{
public:
    using Base = BinaryTree<Trait>;
    using typename Base::Node;
    using typename Base::value_type;

    using Base::Base;
    using Base::insert;

private:
    static int balance_factor(const Node *node){
        if(node == nullptr){
            return 0;
        }
        return Base::template node_height<Node>(node->m_pChild[0])
             - Base::template node_height<Node>(node->m_pChild[1]);
    }

    static Node *rotate_left(Node *node){
        Node *pivot = node->m_pChild[1];
        Node *branch = pivot->m_pChild[0];
        pivot->m_pChild[0] = node;
        node->m_pChild[1] = branch;
        Base::template refresh_node<Node>(node);
        Base::template refresh_node<Node>(pivot);
        return pivot;
    }

    static Node *rotate_right(Node *node){
        Node *pivot = node->m_pChild[0];
        Node *branch = pivot->m_pChild[1];
        pivot->m_pChild[1] = node;
        node->m_pChild[0] = branch;
        Base::template refresh_node<Node>(node);
        Base::template refresh_node<Node>(pivot);
        return pivot;
    }

    static void balance(Node *&node){
        Base::template refresh_node<Node>(node);
        const int factor = balance_factor(node);

        if(factor > 1){
            if(balance_factor(node->m_pChild[0]) < 0){
                node->m_pChild[0] = rotate_left(node->m_pChild[0]);
            }
            node = rotate_right(node);
            return;
        }

        if(factor < -1){
            if(balance_factor(node->m_pChild[1]) > 0){
                node->m_pChild[1] = rotate_right(node->m_pChild[1]);
            }
            node = rotate_left(node);
            return;
        }

        Base::template refresh_node<Node>(node);
    }

public:
    // Adapter insert de BinaryTree
    void insert(const value_type &data, Ref ref) override{
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);
        this->template internal_insert_impl(
            this->m_pRoot,
            data,
            ref,
            [](Node *&node){
                balance(node);
            });
    }
};

#endif // __BINARYTREE_H__
