#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <mutex>        // unique_lock, shared_lock
#include <shared_mutex> // shared_mutex
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
using namespace std;

// Forward iterator
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
public:
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    // TODO 4: Completar el operator++
    MySelf operator++() {this->m_pNode = this->m_pNode->getNext(); return *this; }
};

// Linked List Node
template <typename T>
class LLNode{
    using Node = LLNode<T>;
private:
    T   m_data;
    Ref m_ref;
    Node *m_next;
public:
    LLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data, Ref ref) : m_data(data), m_ref(ref), m_next(nullptr) {}
    LLNode(T data, Ref ref, Node *next) : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Ref    getRef() const { return m_ref; }
    void   setRef(Ref ref) { m_ref = ref; }
    Node*  getNext() const { return m_next; }
    Node*& getNextRef()    { return m_next; }
    void   setNext(Node *next) { m_next = next; }
};

template <typename T>
struct AscendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = greater<T>;
};

template <typename Trait>
class LinkedList{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = LinkedList<Trait>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    // friend forward_iterator;

private:
    Node *m_pRoot = nullptr;
    Node *m_tail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;
public:
    LinkedList() {}
    LinkedList(const LinkedList &other); // Copy constructor
    LinkedList(LinkedList &&other); // Move constructor
    LinkedList& operator=(const LinkedList &other); // Copy assignment operator
    LinkedList& operator=(LinkedList &&other); // Move assignment operator
    
    virtual        ~LinkedList();
    virtual void    push_front(value_type value, Ref ref);
    virtual void    pop_front();
    virtual void    push_back(value_type value, Ref ref);
    virtual void    pop_back();
private:
            void    internal_insert(Node* &pParent, const value_type &value, Ref ref);
public:
    virtual void    insert(const value_type &value, Ref ref);
    
    virtual value_type& operator[](size_t index);
    virtual size_t  size() const;
    virtual string  toString() const;

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    // TODO 12: Agregar Foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        // TODO 13: Concurrent
        unique_lock<shared_mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }
};

// TODO 1: Copy constructor
template <typename Trait>
LinkedList<Trait>::LinkedList(const LinkedList &other){
    m_pRoot = nullptr;
    m_tail = nullptr;
    m_size = 0;

    Node *current = other.m_pRoot;
    while(current != nullptr){
        this->push_back(current->getData(), current->getRef());
        current = current->getNext();
    }
}

// TODO 2: Move constructor
template <typename Trait>
LinkedList<Trait>::LinkedList(LinkedList &&other){
    m_pRoot = move(other.m_pRoot);
    m_tail = move(other.m_tail);
    m_size = move(other.m_size);
    other.m_pRoot = nullptr;
    other.m_tail  = nullptr;
    other.m_size  = 0;
}

// TODO 3: Destructor seguro
template <typename Trait>
LinkedList<Trait>::~LinkedList(){
    Node *current = m_pRoot;
    while(current != nullptr){
        Node *next = current->getNext();
        delete current; 
        current = next;
    }
}

template <typename Trait>
void LinkedList<Trait>::internal_insert(Node* &pPrev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        pPrev = new Node(value, ref, pPrev);
        m_size++;
        if(pPrev->getNext() == nullptr)
            m_tail = pPrev;
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

// TODO 13: Concurrent insert 
template <typename Trait>
void LinkedList<Trait>::insert(const value_type &value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    internal_insert(m_pRoot, value, ref);
}

// TODO 5: push_front
template <typename Trait>
void LinkedList<Trait>::push_front(value_type value, Ref ref){
    // TODO 13: Concurrent 
    unique_lock<shared_mutex> lock(m_mtx);
    m_pRoot = new Node(value, ref, m_pRoot);
    if(m_size == 0)
        m_tail = m_pRoot;
    m_size++;
}

// TODO 6: pop_front
template <typename Trait>
void LinkedList<Trait>::pop_front(){
    // TODO 13: Concurrent
    unique_lock<shared_mutex> lock(m_mtx);
    if(m_size == 0) return;
    Node *temp = m_pRoot;
    m_pRoot = m_pRoot->getNext();
    delete temp;
    m_size--;
    if(m_size == 0)
        m_tail = nullptr;
}

// TODO 7: push_back
template <typename Trait>
void LinkedList<Trait>::push_back(value_type value, Ref ref){
    // TODO 13: Concurrent
    unique_lock<shared_mutex> lock(m_mtx);
    Node *newNode = new Node(value, ref);
    if(m_size == 0){
        m_pRoot = newNode;
        m_tail = newNode;
        m_size++;
        return;
    }
    m_tail->setNext(newNode);
    m_tail = newNode;
    m_size++;
}

// TODO 8: pop_back
template <typename Trait>
void LinkedList<Trait>::pop_back(){
    // TODO 13: Concurrent
    unique_lock<shared_mutex> lock(m_mtx);
    if(m_size == 0) return;
    if(m_pRoot == m_tail){
        delete m_pRoot;
        m_pRoot = nullptr;
        m_tail = nullptr;
        m_size--;
        return;
    } 
    Node *current = m_pRoot;
    while(current->getNext() != m_tail)
        current = current->getNext();

    delete m_tail;
    current->setNext(nullptr);
    m_tail = current;
    m_size--;
}

// TODO 9: operator[]
template <typename Trait>
typename LinkedList<Trait>::value_type& LinkedList<Trait>::operator[](size_t index){
    // TODO 13: Concurrent
    shared_lock<shared_mutex> lock(m_mtx);
    if(index >= m_size)
        throw std::out_of_range("Index out of range");
    Node *current = m_pRoot;
    for(size_t i = 0; i < index; ++i)
        current = current->getNext();
    return current->getDataRef();
}

template <typename Trait>
size_t LinkedList<Trait>::size() const{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size;
}


template <typename Trait>
string LinkedList<Trait>::toString() const{
    // TODO 13: Concurrent
    shared_lock<shared_mutex> lock(m_mtx);
    if(m_size == 0) return "[]";
    ostringstream oss;
    oss << "[";
    Node *current = m_pRoot;
    for(size_t i = 0; i < m_size - 1; ++i){
        oss << current->getData() << ",";
        current = current->getNext();
    }
    oss << current->getData() << "]";
    return oss.str();
}

// TODO 11: operator<<
template <typename Trait>
ostream& operator<<(ostream& os, const LinkedList<Trait>& list){
    return os << list.toString();
}

// TODO 10: operator>>
template <typename Trait>
istream& operator>>(istream& is, LinkedList<Trait>& list){
    typename Trait::value_type value;
    Ref ref;
    while (is >> value >> ref) {
        list.insert(value, ref);
    }
    return is;
}

void LinkedListDemo();
void ListsDemo();

#endif // __LINKEDLIST_H__