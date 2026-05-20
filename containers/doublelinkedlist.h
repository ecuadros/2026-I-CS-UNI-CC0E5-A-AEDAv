#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"
#include <limits>

// Forward Iterator
template <typename Container>
class DoubleLinkedListForwardIterator
    : public general_iterator<Container, DoubleLinkedListForwardIterator<Container>> {
public:
    using MySelf = DoubleLinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() {
        if(this->m_pNode) this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

// Backward Iterator
template <typename Container>
class DoubleLinkedListBackwardIterator
    : public general_iterator<Container, DoubleLinkedListBackwardIterator<Container>> {
public:
    using MySelf = DoubleLinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() {
        if(this->m_pNode) this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

// Nodo doblemente enlazado
template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>> {
    using Node = DLLNode<T>;
public:
    using value_type = T;
private:
    Node *m_pPrev;
public:
    DLLNode() : LLNode<T, DLLNode<T>>(), m_pPrev(nullptr) {}
    DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr)
        : LLNode<T, DLLNode<T>>(data, ref, next), m_pPrev(prev) {}

    Node*  getPrev() const     { return m_pPrev; }
    void   setPrev(Node *prev) { m_pPrev = prev; }
    Node*& getPrevRef()        { return m_pPrev; }
};

// Traits
template <typename T>
struct AscendingDLLTrait : BaseTrait<DLLNode<T>, less<T>> {};

template <typename T>
struct DescendingDLLTrait : BaseTrait<DLLNode<T>, greater<T>> {};

// Lista Doblemente Enlazada
template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait> {
public:
    using value_type        = typename Trait::value_type;
    using Node              = typename Trait::Node;
    using MySelf            = DoubleLinkedList<Trait>;
    using forward_iterator  = DoubleLinkedListForwardIterator<MySelf>;
    using backward_iterator = DoubleLinkedListBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

    DoubleLinkedList() {}

    // Copy constructor
    DoubleLinkedList(const DoubleLinkedList &other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for(Node* curr = other.m_pRoot; curr; curr = curr->getNext())
            push_back(curr->getData(), curr->getRef());
    }

    // Move constructor
    DoubleLinkedList(DoubleLinkedList &&other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = exchange(other.m_pRoot, nullptr);
        this->m_tail  = exchange(other.m_tail,  nullptr);
        this->m_size  = exchange(other.m_size,  0);
    }

    // Copy assignment
    DoubleLinkedList& operator=(const DoubleLinkedList &other) {
        if(this != &other) {
            while(this->m_size > 0) pop_front();
            shared_lock<shared_mutex> lock(other.m_mtx);
            for(Node* curr = other.m_pRoot; curr; curr = curr->getNext())
                push_back(curr->getData(), curr->getRef());
        }
        return *this;
    }

    // Move assignment
    DoubleLinkedList& operator=(DoubleLinkedList &&other) noexcept {
        if(this != &other) {
            while(this->m_size > 0) pop_front();
            unique_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = exchange(other.m_pRoot, nullptr);
            this->m_tail  = exchange(other.m_tail,  nullptr);
            this->m_size  = exchange(other.m_size,  0);
        }
        return *this;
    }

    // Destructor: el de LinkedList (virtual) maneja la eliminación con unique_lock
    virtual ~DoubleLinkedList() {}

    // push_front: mantiene m_pPrev del antiguo root
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, this->m_pRoot, nullptr);
        if(this->m_pRoot) this->m_pRoot->setPrev(newNode);
        this->m_pRoot = newNode;
        if(this->m_size == 0) this->m_tail = newNode;
        this->m_size++;
    }

    // push_back: asigna m_pPrev del nuevo tail
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, nullptr, this->m_tail);
        if(this->m_tail) this->m_tail->setNext(newNode);
        else             this->m_pRoot = newNode;
        this->m_tail = newNode;
        this->m_size++;
    }

    // pop_front: actualiza m_pPrev del nuevo root, retorna tuple
    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot) throw runtime_error("La lista esta vacia");
        Node* temp = this->m_pRoot;
        auto result = make_tuple(temp->getData(), temp->getRef());
        this->m_pRoot = this->m_pRoot->getNext();
        if(this->m_pRoot) this->m_pRoot->setPrev(nullptr);
        else              this->m_tail = nullptr;
        delete temp;
        this->m_size--;
        return result;
    }

    // pop_back: retorna tuple
    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot) throw runtime_error("La lista esta vacia");
        Node* temp = this->m_tail;
        auto result = make_tuple(temp->getData(), temp->getRef());
        this->m_tail = this->m_tail->getPrev();
        if(this->m_tail) this->m_tail->setNext(nullptr);
        else             this->m_pRoot = nullptr;
        delete temp;
        this->m_size--;
        return result;
    }

    // insert ordenado: reimplementado para mantener m_pPrev
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        dll_insert(this->m_pRoot, nullptr, value, ref);
    }

    // Mejora libre #1: operator[] bidireccional — busca desde el extremo más cercano
    value_type& operator[](size_t index) override {
        shared_lock<shared_mutex> lock(this->m_mtx);
        if(index >= this->m_size) throw out_of_range("Indice fuera de rango");
        Node* curr;
        if(index < this->m_size / 2) {
            curr = this->m_pRoot;
            for(size_t i = 0; i < index; ++i) curr = curr->getNext();
        } else {
            curr = this->m_tail;
            for(size_t i = this->m_size - 1; i > index; --i) curr = curr->getPrev();
        }
        return curr->getDataRef();
    }

    forward_iterator  begin()  { return forward_iterator (this, this->m_pRoot); }
    forward_iterator  end()    { return forward_iterator (this, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(this->m_size == 0) return;
        for(auto it = rbegin(); it != rend(); ++it)
            func(*it, forward<Args>(args)...);
    }

private:
    // Inserción ordenada recursiva que mantiene m_pPrev
    void dll_insert(Node* &pNode, Node* prev, const value_type &value, Ref ref) {
        if(!pNode || this->m_comp(value, pNode->getDataRef())) {
            Node* oldNode = pNode;
            pNode = new Node(value, ref, oldNode, prev);
            if(oldNode) oldNode->setPrev(pNode);
            else        this->m_tail = pNode;
            this->m_size++;
            return;
        }
        dll_insert(pNode->getNextRef(), pNode, value, ref);
    }
};

#endif // __DOUBLELINKEDLIST_H__
