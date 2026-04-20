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
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    // TODO: Completar el operator++
    MySelf& operator++() {
        if(this->m_pNode)
            this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
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
    LLNode() : m_data(T()), m_ref(0), m_next(nullptr) {}
    LLNode(T data) : m_data(data), m_ref(0), m_next(nullptr) {}
    LLNode(T data, Ref ref, Node *next) : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Node*  getNext() const { return m_next; }
    Node*& getNextRef()    { return m_next; }
    void   setNext(Node *next) { m_next = next; }
    Ref    getRef() const { return m_ref; }
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
    LinkedList() {};
    LinkedList(const LinkedList &other);
    LinkedList(LinkedList &&other) noexcept;
    LinkedList& operator=(const LinkedList &other);
    LinkedList& operator=(LinkedList &&other) noexcept;

    virtual        ~LinkedList();
    virtual void    push_front(value_type value, Ref ref);
    virtual void    pop_front();
    virtual void    push_back(value_type value, Ref ref);
    virtual void    pop_back();
private:
            void    internal_insert(Node* &pParent, const value_type &value, Ref ref);
    void            clear();
public:
    virtual void    insert(const value_type &value, Ref ref);

    virtual value_type& operator[](size_t index);
    virtual size_t  size() const;
    virtual string  toString() const;

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    // Agregar Foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        unique_lock<shared_mutex> lock(m_mtx);
        for(Node* curr= m_pRoot; curr != nullptr; curr = curr->getNext()) {
            func(curr->getData(), curr->getRef(), forward<Args>(args)... );
        }
    }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args) const {
        shared_lock<shared_mutex> lock(m_mtx);
        //::ForEach(begin(), end(), func, std::forward<Args>(args)... );
        for(Node* curr= m_pRoot; curr != nullptr; curr = curr->getNext()) {
            func(curr->getData(), curr->getRef(), forward<Args>(args)... );
        }
    }
};

template <typename Trait>
void LinkedList<Trait>::internal_insert(Node* &pPrev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        pPrev = new Node(value, ref, pPrev);
        m_size++;
        if(pPrev == m_pRoot)
            m_tail = pPrev;
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename Trait>
void LinkedList<Trait>::insert(const value_type &value, Ref ref){
    internal_insert(m_pRoot, value, ref);
}

template <typename Trait>
void LinkedList<Trait>::push_front(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    Node *newNode = new Node(value, ref, m_pRoot);
    m_pRoot = newNode;
    if (!m_tail) {
        m_tail = newNode;
    }
    m_size++;
}

template <typename Trait>
void LinkedList<Trait>::pop_front(){
    unique_lock<shared_mutex> lock(m_mtx);
    if (!m_pRoot) return;
    Node *tempNode = m_pRoot;
    m_pRoot = m_pRoot->getNext();
    m_size--;
    if (m_size == 0) {
        m_tail = nullptr;
    }
    delete tempNode;
}

template <typename Trait>
void LinkedList<Trait>::push_back(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    Node *newNode = new Node(value, ref, nullptr);
    if (m_tail) {
        m_tail->setNext(newNode);
        m_tail = newNode;
    } else {
        m_pRoot = newNode;
        m_tail = newNode;
    }
    m_size++;
}

template <typename Trait>
void LinkedList<Trait>::pop_back(){
    unique_lock<shared_mutex> lock(m_mtx);
    if (!m_pRoot) return;
    if (m_pRoot == m_tail) {
        delete m_pRoot;
        m_pRoot = m_tail = nullptr;
    } else {
        Node *penultimate = m_pRoot;
        while (penultimate->getNext() != m_tail) {
            penultimate = penultimate->getNext();
        }
        delete m_tail;
        m_tail = penultimate;
        m_tail->setNext(nullptr);
    }
    m_size--;
}

template <typename Trait>
LinkedList<Trait>::LinkedList(LinkedList &&other) noexcept {
    unique_lock<shared_mutex> lock(other.m_mtx);
    this->m_pRoot = other.m_pRoot;
    this->m_tail = other.m_tail;
    this->m_size = other.m_size;
    this->m_comp = move(other.m_comp);

    // liberamos el objeto original
    other.m_pRoot = other.m_tail = nullptr;
    other.m_size = 0;

}

template <typename Trait>
LinkedList<Trait>::LinkedList(const LinkedList &other) {
    this->m_pRoot = nullptr;
    this->m_tail = nullptr;
    this->m_size = 0;
    this->m_comp = other.m_comp;

    shared_lock<shared_mutex> lock(other.m_mtx);
    for (Node* current = other.m_pRoot; current != nullptr; current = current->getNext()) {
        this->push_back(current->getData(), current->getRef());
    }
}

template <typename Trait>
void LinkedList<Trait>::clear() {
    unique_lock<shared_mutex> lock(m_mtx);
    while(m_pRoot){
        Node *temp = m_pRoot;
        m_pRoot = m_pRoot->getNext();
        delete temp;
    }
    m_tail = nullptr;
    m_size = 0;
}

template <typename Trait>
LinkedList<Trait>::~LinkedList() {
    clear();
}

template <typename Trait>
LinkedList<Trait>& LinkedList<Trait>::operator=(const LinkedList &other) {
    if (this != &other) {
        clear();
        this->m_comp = other.m_comp;

        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node* current = other.m_pRoot; current != nullptr; current = current->getNext()) {
            this->push_back(current->getData(), current->getRef());
        }
    }
    return *this;
}

template <typename Trait>
LinkedList<Trait>& LinkedList<Trait>::operator=(LinkedList &&other) noexcept {
    if (this != &other) {
        clear();
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = other.m_pRoot;
        this->m_tail = other.m_tail;
        this->m_size = other.m_size;
        this->m_comp = move(other.m_comp);

        // liberamos el objeto original
        other.m_pRoot = other.m_tail = nullptr;
        other.m_size = 0;
    }
    return *this;
}

template <typename Trait>
typename Trait::value_type& LinkedList<Trait>::operator[](size_t index) {
    shared_lock<shared_mutex> lock(m_mtx);
    if (index >= m_size) {
        throw out_of_range("Index out of range");
    }
    Node* current = m_pRoot;
    for (size_t i = 0; i < index; ++i) {
        current = current->getNext();
    }
    return current->getDataRef();
}

template <typename Trait>
string LinkedList<Trait>::toString() const {
    ostringstream oss;
    oss << "[";
    bool first = true;

    this->ForEach([&](const value_type& value, Ref ref) {
        if (!first) {
            oss << "->";
        }
        oss << "(" << value << ", " << ref << ")";
        first = false;
    });
    oss << "]";
    return oss.str();
}

template <typename Trait>
ostream& operator<<(ostream& os, const LinkedList<Trait>& list){
    return os << list.toString();
}

template <typename Trait>
istream& operator>>(istream& is, LinkedList<Trait>& list){
    typename Trait::value_type value;
    Ref ref;
    while (is >> value >> ref) {
        list.insert(value, ref);
    }
    return is;
}

template <typename Trait>
size_t LinkedList<Trait>::size() const {
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size;
}

#endif // __LINKEDLIST_H__
