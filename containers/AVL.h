#ifndef __AVL_H__
#define __AVL_H__

#include "BinaryTree.h"

// AVL Node: se le agrega la altura (m_height)
template <typename T>
class AVLNode : public BinaryTreeNode<T>{
public:
    using value_type = T;
    using Node = BinaryTreeNode<T>;
private:
    int m_height;
public:
    AVLNode() : BinaryTreeNode<T>(), m_height(1) {}
    AVLNode(T data, Ref ref) : BinaryTreeNode<T>(data, ref), m_height(1) {}
    AVLNode(T data, Ref ref, Node *parent) : BinaryTreeNode<T>(data, ref, parent), m_height(1) {}
    virtual ~AVLNode() {}

    int  getHeight() const { return m_height; }
    void setHeight(int h)  { m_height = h; }
};

// Traits de Ordenamiento
template <typename T>
struct AscendingAVLTrait : public BaseTrait<BinaryTreeNode<T>, less<T>>{
};

template <typename T>
struct DescendingAVLTrait : public BaseTrait<BinaryTreeNode<T>, greater<T>>{
};


// Contenedor AVL, se reutiliza la gran parte del codigo de BinaryTree
template <typename Trait>
class AVL : public BinaryTree<Trait>{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = AVL<Trait>;

    AVL() : BinaryTree<Trait>() {}

    // Copy Constructor (clona con AVLNode + alturas)
    AVL(const AVL &other) : BinaryTree<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = internal_clone(other.m_pRoot, nullptr);
        this->m_size  = other.m_size;
    }

    // Move Constructor
    AVL(AVL &&other) : BinaryTree<Trait>() {
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_size  = std::exchange(other.m_size,  0);
    }

    // Copy Assignment
    AVL& operator=(const AVL &other) {
        if (this != &other) {
            unique_lock<shared_mutex> lockMe(this->m_mtx);
            this->internal_destroy(this->m_pRoot);
            this->m_pRoot = nullptr;
            this->m_size  = 0;
            shared_lock<shared_mutex> lockOther(other.m_mtx);
            this->m_pRoot = internal_clone(other.m_pRoot, nullptr);
            this->m_size  = other.m_size;
        }
        return *this;
    }

    // Move Assignment
    AVL& operator=(AVL &&other) {
        if (this != &other) {
            unique_lock<shared_mutex> lockMe(this->m_mtx);
            this->internal_destroy(this->m_pRoot);
            unique_lock<shared_mutex> lockOther(other.m_mtx);
            this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
            this->m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    // ~AVL: se reusa el destructor virtual de BinaryTree (internal_destroy

protected:
    int   internal_height(Node *n);
    void  internal_update_height(Node *n);
    int   internal_balance(Node *n);
    void  internal_rotate_left(Node *&pNode);
    void  internal_rotate_right(Node *&pNode);
    void  internal_insert(Node *&pNode, Node *parent, const value_type &value, Ref ref) override;
    Node* internal_clone(Node *src, Node *parent) override;
};

// Implementacion de Metodos del Arbol
template <typename Trait>
int AVL<Trait>::internal_height(Node *n){
    return n == nullptr ? 0 : static_cast<AVLNode<value_type>*>(n)->getHeight();
}

template <typename Trait>
void AVL<Trait>::internal_update_height(Node *n){
    int hl = internal_height(n->getChild(0));
    int hr = internal_height(n->getChild(1));
    static_cast<AVLNode<value_type>*>(n)->setHeight(1 + (hl > hr ? hl : hr));
}

template <typename Trait>
int AVL<Trait>::internal_balance(Node *n){
    return internal_height(n->getChild(0)) - internal_height(n->getChild(1));
}

template <typename Trait>
void AVL<Trait>::internal_rotate_left(Node *&pNode){
    Node *x   = pNode->getChild(1);
    Node *par = pNode->getParent();
    pNode->setChild(1, x->getChild(0));
    if (x->getChild(0)){
        x->getChild(0)->setParent(pNode);
    }
    x->setChild(0, pNode);
    pNode->setParent(x);
    x->setParent(par);
    internal_update_height(pNode);
    internal_update_height(x);
    pNode = x;
}

template <typename Trait>
void AVL<Trait>::internal_rotate_right(Node *&pNode){
    Node *x   = pNode->getChild(0);
    Node *par = pNode->getParent();
    pNode->setChild(0, x->getChild(1));
    if (x->getChild(1)) {
        x->getChild(1)->setParent(pNode);
    }
    x->setChild(1, pNode);
    pNode->setParent(x);
    x->setParent(par);
    internal_update_height(pNode);
    internal_update_height(x);
    pNode = x;
}

// Codigo adaptado del internal_insert de BinaryTree
template <typename Trait>
void AVL<Trait>::internal_insert(Node *&pNode, Node *parent, const value_type &value, Ref ref){
    if (pNode == nullptr) {
        pNode = new AVLNode<value_type>(value, ref, parent);
        this->m_size++;
        return;
    }
    auto branch = !this->m_comp(value, pNode->getData());
    internal_insert(pNode->getChildRef(branch), pNode, value, ref);

    internal_update_height(pNode);

    int hb = internal_balance(pNode);
    if (hb > 1){
        if (internal_balance(pNode->getChild(0)) < 0){
            internal_rotate_left(pNode->getChildRef(0));
        }
        internal_rotate_right(pNode);
    }
    else if (hb < -1) {
        if (internal_balance(pNode->getChild(1)) > 0){
            internal_rotate_right(pNode->getChildRef(1));
        }
        internal_rotate_left(pNode);
    }
}

template <typename Trait>
typename AVL<Trait>::Node*
AVL<Trait>::internal_clone(Node *src, Node *parent){
    if (!src) return nullptr;
    Node *n = new AVLNode<value_type>(src->getData(), src->getRef(), parent);
    n->setChild(0, internal_clone(src->getLeft(),  n));
    n->setChild(1, internal_clone(src->getRight(), n));
    internal_update_height(n);
    return n;
}

#endif
