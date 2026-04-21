#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <shared_mutex> // shared_mutex
#include <mutex>        // unique_lock
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
    // T4: operator++ del iterador
    MySelf operator++() { 
        this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

// Linked List Node
template <typename T>
class LLNode{
    using Node = LLNode<T>;
private:
    T    m_data;
    Ref  m_ref;
    Node *m_next;
public:
    LLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data) : m_data(data), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data, Node *next) : m_data(data), m_ref(Ref()), m_next(next) {}
    LLNode(T data, Ref ref, Node *next) : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Ref    getRef()  const { return m_ref; }
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
    friend forward_iterator; // T13: concurrencia — acceso a m_pRoot desde el iterador

private:
    Node *m_pRoot = nullptr;
    Node *m_tail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;
public:
    LinkedList() {}
    // T1: Copy constructor
    LinkedList(const LinkedList &other){ 
        shared_lock<shared_mutex> lock(other.m_mtx);
        Node *current = other.m_pRoot;
        while(current){
            Node *newNode = new Node(current->getData(), current->getRef(), nullptr);
            if(!m_pRoot)
                m_pRoot = m_tail = newNode;
            else{
                m_tail->setNext(newNode);
                m_tail = newNode;
            }
            m_size++;
            current = current->getNext();
        }
    }
    // T2: Move constructor
    LinkedList(LinkedList &&other){
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = other.m_pRoot;
        m_tail = other.m_tail;
        m_size = other.m_size;
        other.m_pRoot = nullptr;
        other.m_tail = nullptr;
        other.m_size = 0;
    }
    LinkedList& operator=(const LinkedList &other){ // Copy assignment operator
        return *this;
    }
    LinkedList& operator=(LinkedList &&other){ // Move assignment operator
        return *this;
    }
    // T3: Destructor seguro
    virtual ~LinkedList() {
        Node *current = m_pRoot;
        while(current){
            Node *next = current->getNext();
            delete current;
            current = next;
        }
    }
    virtual void    push_front(value_type value, Ref ref); // T5: push_front
    virtual void    pop_front();                           // T6: pop_front
    virtual void    push_back(value_type value, Ref ref);  // T7: push_back
    virtual void    pop_back();                            // T8: pop_back
private:
            void    internal_insert(Node* &pParent, const value_type &value, Ref ref);
public:
    virtual void    insert(const value_type &value, Ref ref);

    virtual value_type& operator[](size_t index);         // T9: operator[]
    virtual size_t  size() const;                         // T10: size
    virtual string  toString() const;                     // T11: toString + operator<<

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    // Agregar Foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        unique_lock<shared_mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }
};

// T5: push_front
template <typename Trait>
void LinkedList<Trait>::push_front(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    Node *newNode = new Node(value, ref, m_pRoot);
    m_pRoot = newNode;
    if(m_tail == nullptr) m_tail = m_pRoot;
    m_size++;
}

// T6: pop_front
template <typename Trait>
void LinkedList<Trait>::pop_front(){
    unique_lock<shared_mutex> lock(m_mtx);
    if(m_pRoot == nullptr) return;
    Node *toDelete = m_pRoot;
    m_pRoot = m_pRoot->getNext();
    if(m_pRoot == nullptr) m_tail = nullptr;
    delete toDelete;
    m_size--;
}

// T7: push_back
template <typename Trait>
void LinkedList<Trait>::push_back(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    Node *newNode = new Node(value, ref, nullptr);
    if(m_tail == nullptr)
        m_pRoot = m_tail = newNode;
    else{
        m_tail->setNext(newNode);
        m_tail = newNode;
    }
    m_size++;
}

// T8: pop_back
template <typename Trait>
void LinkedList<Trait>::pop_back(){
    unique_lock<shared_mutex> lock(m_mtx);
    if(m_pRoot == nullptr) return;
    if(m_pRoot == m_tail){
        delete m_pRoot;
        m_pRoot = m_tail = nullptr;
        m_size--;
        return;
    }
    Node *current = m_pRoot;
    while(current->getNext() != m_tail)
        current = current->getNext();
    delete m_tail;
    m_tail = current;
    m_tail->setNext(nullptr);
    m_size--;
}

// T10: size
template <typename Trait>
size_t LinkedList<Trait>::size() const{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size;
}

// T9: operator[]
template <typename Trait>
typename LinkedList<Trait>::value_type& LinkedList<Trait>::operator[](size_t index){
    shared_lock<shared_mutex> lock(m_mtx);
    Node *current = m_pRoot;
    for(size_t i = 0; i < index; ++i)
        current = current->getNext();
    return current->getDataRef();
}

template <typename Trait>
void LinkedList<Trait>::internal_insert(Node* &pPrev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        Node* newNode = new Node(value, ref, pPrev);
        if(pPrev == nullptr) m_tail = newNode;
        pPrev = newNode;
        m_size++;
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename Trait>
void LinkedList<Trait>::insert(const value_type &value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx); // T13: concurrencia
    internal_insert(m_pRoot, value, ref);
}

// T11: toString
template <typename Trait>
string LinkedList<Trait>::toString() const{
    shared_lock<shared_mutex> lock(m_mtx);
    ostringstream oss;
    oss << "(";
    Node *current = m_pRoot;
    bool first = true;
    while(current){
        if(!first) oss << "->";
        oss << current->getData();
        first = false;
        current = current->getNext();
    }
    oss << ")";
    return oss.str();
}

// T11: operator<<
template <typename Trait>
ostream& operator<<(ostream& os, const LinkedList<Trait>& list){
    return os << list.toString();
}

// T12: operator>>
template <typename Trait>
istream& operator>>(istream& is, LinkedList<Trait>& list){
    using value_type = typename LinkedList<Trait>::value_type;
    char ch;
    is >> ch;
    if(!is || ch != '[') { is.setstate(ios::failbit); return is; }
    if((is >> ws).peek() == ']') return is >> ch;
    value_type data;
    while(is >> data){
        list.insert(data, Ref());
        is >> ch;
        if(ch == ']') break;
        else if(ch != ',') { is.setstate(ios::failbit); break; }
    }
    return is;
}

void ListsDemo();

#endif // __LINKEDLIST_H__