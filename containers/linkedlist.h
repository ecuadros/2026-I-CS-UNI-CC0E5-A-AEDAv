#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex> 
#include <utility>
#include <tuple>
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
#include "traits.h"
using namespace std;

// Iterador Forward
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
public:
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    
    MySelf operator++() {
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getNext();
        }
        return *this;
    }
};

// Iterador Backward (solo listas con getPrev: DLL, CDLL)
template <typename Container>
class LinkedListBackwardIterator : public general_iterator<Container, LinkedListBackwardIterator<Container>>{
public:
    using MySelf = LinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++() {
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getPrev();
        }
        return *this;
    }
};

// Linked List Node
template <typename T, typename NodeType = void>
class LLNode{
protected:
    using Node = conditional_t<is_void_v<NodeType>, LLNode, NodeType>;
private:
    T   m_data;
    Ref m_ref;
    Node *m_next;
public:
    using value_type = T;
    LLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data, Ref ref) : m_data(data), m_ref(ref), m_next(nullptr) {}
    LLNode(T data, Ref ref, Node *next) : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Ref    getRef() const  { return m_ref; }
    void   setRef(Ref ref) { m_ref = ref; }
    Node* getNext() const { return m_next; }
    Node*& getNextRef()    { return m_next; }
    void   setNext(Node *next) { m_next = next; }
};

// Traits de Ordenamiento
template <typename T>
struct AscendingLinkedListTrait  : public BaseTrait<LLNode<T>, less<T>>{
};

template <typename T>
struct DescendingLinkedListTrait : public BaseTrait<LLNode<T>, greater<T>>{
};


// Contenedor Principal LinkedList
template <typename Trait>
class LinkedList{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = LinkedList<Trait>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    friend forward_iterator;

protected:
    Node *m_pRoot = nullptr;
    Node *m_tail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;

private:
    void internal_insert(Node* &pPrev, const value_type &value, Ref ref);

public:
    LinkedList() {}
    
    // Copy Constructor
    LinkedList(const LinkedList &other) : m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        shared_lock<shared_mutex> lock(other.m_mtx); 
        for (Node* curr = other.m_pRoot; curr != nullptr; curr = curr->getNext()) {
            push_back(curr->getData(), curr->getRef());
        }
    }
    
    // Move Constructor
    LinkedList(LinkedList &&other) : m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        unique_lock<shared_mutex> lockOther(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_tail  = std::exchange(other.m_tail, nullptr);
        m_size  = std::exchange(other.m_size, 0);
    }

    // Copy Assignment
    LinkedList& operator=(const LinkedList &other) {
        if (this != &other) {
            while (m_size > 0) pop_front(); 
            shared_lock<shared_mutex> lock(other.m_mtx);
            for (Node* curr = other.m_pRoot; curr != nullptr; curr = curr->getNext()) {
                push_back(curr->getData(), curr->getRef());
            }
        }
        return *this;
    }
    
    // Move Assignment
    LinkedList& operator=(LinkedList &&other) {
        if (this != &other) {
            while (m_size > 0) pop_front(); 
            unique_lock<shared_mutex> lockOther(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_tail  = std::exchange(other.m_tail, nullptr);
            m_size  = std::exchange(other.m_size, 0);
        }
        return *this;
    }
    
    // Destructor Seguro
    virtual ~LinkedList() {
        unique_lock<shared_mutex>lock(m_mtx);
        Node* current = m_pRoot;
        while(current){
            Node* next = current->getNext();
            delete current;
            current = next;
        }
        m_pRoot = nullptr;
        m_tail = nullptr;
        m_size = 0;
    }

    // Operaciones
    virtual void    push_front(value_type value, Ref ref);
    virtual std::tuple<value_type, Ref> pop_front();
    virtual void    push_back(value_type value, Ref ref);
    virtual std::tuple<value_type, Ref> pop_back();
    virtual void    insert(const value_type &value, Ref ref);
    
    virtual value_type& operator[](size_t index);
    virtual size_t  size() const;

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    // ForEach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        for(auto& item : *this) {
            func(item, std::forward<Args>(args)...);
        }
    }

    // Operadores I/O
    friend ostream& operator<<(ostream& os, const LinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        list.do_print(os); 
        os << "]";
        return os;
    }

    friend istream& operator>>(istream& is, LinkedList& list) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return is;
        }
        value_type val;
        Ref ref;
        char comma, parenClose;
        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref >> parenClose) {
                    if (comma == ',' && parenClose == ')') {
                        list.push_back(val, ref);  // virtual → llama al push_back correcto
                    }
                }
            }
        }
        return is;
    }

protected:
    virtual void do_print(ostream& os) const {
        Node* act = m_pRoot;
        while (act) {
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (act->getNext()) os << ",";
            act = act->getNext();
        }
    }
};

// Implementacion de Metodos de Lista
template <typename Trait>
void LinkedList<Trait>::internal_insert(Node* &pPrev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        pPrev = new Node(value, ref, pPrev);
        m_size++;
        if(pPrev->getNext() == nullptr){
            m_tail = pPrev;
        }
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename Trait>
void LinkedList<Trait>::insert(const value_type &value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx); 
    internal_insert(m_pRoot, value, ref);
    if(m_size == 1){
        m_tail = m_pRoot;
    }else{
        Node* act = m_pRoot;
        while(act && act->getNext()){
            act = act->getNext();
        }
        m_tail = act;
    }
}

// PushFront
template <typename Trait>
void LinkedList<Trait>::push_front(value_type value, Ref ref) {
    unique_lock<shared_mutex> lock(m_mtx); 
    m_pRoot = new Node(value, ref, m_pRoot);
    if(m_size == 0){
        m_tail = m_pRoot;
    }
    m_size++;
}

// PushBack
template <typename Trait>
void LinkedList<Trait>::push_back(value_type value, Ref ref) {
    unique_lock<shared_mutex> lock(m_mtx); 
    Node* newNode = new Node(value, ref);
    if(m_size == 0){
        m_pRoot = newNode;
        m_tail = newNode;
    }else{
        m_tail->setNext(newNode);
        m_tail = newNode;
    }
    m_size++;
}

// pop_front: Retornar par de datos (Data y Ref) de la cabeza y eliminar el nodo
template <typename Trait>
std::tuple<typename LinkedList<Trait>::value_type, Ref> LinkedList<Trait>::pop_front() {
    unique_lock<shared_mutex> lock(m_mtx); 
    if (!m_pRoot) throw runtime_error("La lista esta vacia");
    
    Node* temp = m_pRoot;
    auto result = std::make_tuple(temp->getData(), temp->getRef());
    
    m_pRoot = m_pRoot->getNext();
    delete temp;
    m_size--;
    
    if (m_size == 0) m_tail = nullptr;
    return result;
}

// pop_back: Retornar par de datos (Data y Ref) del final y eliminar el nodo
template <typename Trait>
std::tuple<typename LinkedList<Trait>::value_type, Ref> LinkedList<Trait>::pop_back() {
    unique_lock<shared_mutex> lock(m_mtx); 
    if (!m_pRoot) throw runtime_error("La lista esta vacia");
    
    std::tuple<value_type, Ref> result;

    if (m_pRoot == m_tail) {
        result = std::make_tuple(m_pRoot->getData(), m_pRoot->getRef());
        delete m_pRoot;
        m_pRoot = m_tail = nullptr;
    } else {
        Node* act = m_pRoot;
        while (act->getNext() != m_tail) {
            act = act->getNext();
        }
        result = std::make_tuple(m_tail->getData(), m_tail->getRef());
        delete m_tail;
        m_tail = act;
        m_tail->setNext(nullptr);
    }
    m_size--;
    return result;
}

template <typename Trait>
typename LinkedList<Trait>::value_type& LinkedList<Trait>::operator[](size_t index) {
    shared_lock<shared_mutex> lock(m_mtx); 
    if (index >= m_size) throw out_of_range("Indice fuera de rango");
    Node* act = m_pRoot;
    for (size_t i = 0; i < index; ++i) {
        act = act->getNext();
    }
    return act->getDataRef();
}

template <typename Trait>
size_t LinkedList<Trait>::size() const {
    shared_lock<shared_mutex> lock(m_mtx); 
    return m_size;
}

#endif // __LINKEDLIST_H__