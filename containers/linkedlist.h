#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <cstddef> // size_t
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include "shared_mutex_compat.h"
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
using namespace std;

using linked_list_shared_mutex = compat_shared_mutex;

// Forward iterator
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
public:
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    // T4: impl forward it 
    MySelf &operator++() {
        if(this->m_pNode != nullptr)
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
    LLNode(T data, Ref ref, Node *next = nullptr)
        : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Ref    getRef() const  { return m_ref; }
    void   setRef(Ref ref) { m_ref = ref; }
    Node*  getNext() const { return m_next; }
    Node*& getNextRef()    { return m_next; }
    void   setNext(Node *next) { m_next = next; }
};

template <typename T>
ostream& operator<<(ostream& os, const LLNode<T>& node){
    return os << "(" << node.getData() << ", " << node.getRef() << ")";
}

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

private:
    Node *m_pRoot = nullptr;
    Node *m_tail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;

    // T13: concurrencia 
    mutable linked_list_shared_mutex m_mtx;

public:
    LinkedList() = default;
    LinkedList(const LinkedList &other);
    LinkedList(LinkedList &&other) noexcept;
    LinkedList& operator=(const LinkedList &other);
    LinkedList& operator=(LinkedList &&other) noexcept;
    virtual ~LinkedList();
    virtual void push_front(value_type value, Ref ref);
    virtual void pop_front();
    virtual void push_back(value_type value, Ref ref);
    virtual void pop_back();
private:
    void internal_insert(Node* &pParent, const value_type &value, Ref ref);
public:
    virtual void insert(const value_type &value, Ref ref);
    virtual value_type& operator[](size_t index);

    virtual size_t  size() const;
    virtual string  toString() const;
    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }
    // T12: Foreach 
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args){
        unique_lock<linked_list_shared_mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }
};

template <typename Trait>
// T1: Copy constructor
LinkedList<Trait>::LinkedList(const LinkedList &other){
    shared_lock<linked_list_shared_mutex> lock(other.m_mtx);
    Node *current = other.m_pRoot;

    while(current != nullptr){
        Node *new_node = new Node(current->getData(), current->getRef());
        if(m_pRoot == nullptr){
            m_pRoot = new_node;
            m_tail = new_node;
        }
        else{
            m_tail->setNext(new_node);
            m_tail = new_node;
        }
        ++m_size;
        current = current->getNext();
    }
}

template <typename Trait>
// T2: Move constructor
LinkedList<Trait>::LinkedList(LinkedList &&other) noexcept{
    unique_lock<linked_list_shared_mutex> lock(other.m_mtx);
    m_pRoot = other.m_pRoot;
    m_tail = other.m_tail;
    m_size = other.m_size;

    other.m_pRoot = nullptr;
    other.m_tail = nullptr;
    other.m_size = 0;
}

template <typename Trait>
LinkedList<Trait>& LinkedList<Trait>::operator=(const LinkedList &other){
    if(this == &other)
        return *this;

    shared_lock<linked_list_shared_mutex> other_lock(other.m_mtx);
    unique_lock<linked_list_shared_mutex> lock(m_mtx);

    while(m_pRoot != nullptr){
        Node *temp = m_pRoot;
        m_pRoot = m_pRoot->getNext();
        delete temp;
    }

    m_tail = nullptr;
    m_size = 0;

    Node *current = other.m_pRoot;
    while(current != nullptr){
        Node *new_node = new Node(current->getData(), current->getRef());

        if(m_pRoot == nullptr){
            m_pRoot = new_node;
            m_tail = new_node;
        }
        else{
            m_tail->setNext(new_node);
            m_tail = new_node;
        }
        ++m_size;
        current = current->getNext();
    }

    
    return *this;
}

template <typename Trait>
LinkedList<Trait>& LinkedList<Trait>::operator=(LinkedList &&other) noexcept{
    if(this == &other)
        return *this;

    unique_lock<linked_list_shared_mutex> other_lock(other.m_mtx);
    unique_lock<linked_list_shared_mutex> lock(m_mtx);

    while(m_pRoot != nullptr){
        Node *temp = m_pRoot;
        m_pRoot = m_pRoot->getNext();
        delete temp;
    }

    m_pRoot = other.m_pRoot;
    m_tail = other.m_tail;
    m_size = other.m_size;

    other.m_pRoot = nullptr;
    other.m_tail = nullptr;
    other.m_size = 0;

    return *this;
}

template <typename Trait>
// T3: Destructor seguro
LinkedList<Trait>::~LinkedList(){
    unique_lock<linked_list_shared_mutex> lock(m_mtx);

    while(m_pRoot != nullptr){
        Node *temp = m_pRoot;
        m_pRoot = m_pRoot->getNext();
        delete temp;
    }

    m_tail = nullptr;
    m_size = 0;
}

template <typename Trait>
// T5: push_front
void LinkedList<Trait>::push_front(value_type value, Ref ref){
    unique_lock<linked_list_shared_mutex> lock(m_mtx);

    Node *new_node = new Node(value, ref, m_pRoot);
    m_pRoot = new_node;
    if(m_tail == nullptr)
        m_tail = new_node;
    ++m_size;
}

template <typename Trait>
// T6: pop_front
void LinkedList<Trait>::pop_front(){
    unique_lock<linked_list_shared_mutex> lock(m_mtx);

    if(m_pRoot == nullptr)
        return;

    Node *old_root = m_pRoot;
    m_pRoot = m_pRoot->getNext();
    delete old_root;
    --m_size;

    if(m_pRoot == nullptr)
        m_tail = nullptr;
}

template <typename Trait>
// T7: push_back
void LinkedList<Trait>::push_back(value_type value, Ref ref){
    unique_lock<linked_list_shared_mutex> lock(m_mtx);

    Node *new_node = new Node(value, ref);
    if(m_tail == nullptr)
        m_pRoot = m_tail = new_node;
    else{
        m_tail->setNext(new_node);
        m_tail = new_node;
    }
    ++m_size;
}

template <typename Trait>
// T8: pop_back
void LinkedList<Trait>::pop_back(){
    unique_lock<linked_list_shared_mutex> lock(m_mtx);

    if(m_tail == nullptr)
        return;

    if(m_pRoot == m_tail){
        delete m_tail;
        m_pRoot = nullptr;
        m_tail = nullptr;
        m_size = 0;
        return;
    }

    Node *previous = m_pRoot;
    while(previous->getNext() != m_tail)
        previous = previous->getNext();

    delete m_tail;
    m_tail = previous;
    m_tail->setNext(nullptr);
    --m_size;
}

template <typename Trait>
void LinkedList<Trait>::internal_insert(Node* &pParent, const value_type &value, Ref ref){
    if(pParent == nullptr || m_comp(value, pParent->getDataRef())){
        pParent = new Node(value, ref, pParent);
        ++m_size;
        if(pParent->getNext() == nullptr)
            m_tail = pParent;
        return;
    }
    internal_insert(pParent->getNextRef(), value, ref);
}

template <typename Trait>
void LinkedList<Trait>::insert(const value_type &value, Ref ref){
    unique_lock<linked_list_shared_mutex> lock(m_mtx);
    internal_insert(m_pRoot, value, ref);
}

template <typename Trait>
// T9: operator[]
typename LinkedList<Trait>::value_type& LinkedList<Trait>::operator[](size_t index){
    unique_lock<linked_list_shared_mutex> lock(m_mtx);
    if(index >= m_size)
        throw out_of_range("LinkedList index out of range");
    Node *current = m_pRoot;
    for(size_t i = 0; i < index; ++i)
        current = current->getNext();
    return current->getDataRef();
}
template <typename Trait>
size_t LinkedList<Trait>::size() const{
    shared_lock<linked_list_shared_mutex> lock(m_mtx);
    return m_size;
}
template <typename Trait>
string LinkedList<Trait>::toString() const{
    shared_lock<linked_list_shared_mutex> lock(m_mtx);

    ostringstream oss;
    oss << "[";
    Node *current = m_pRoot;
    bool first = true;
    while(current != nullptr){
        if(!first)
            oss << ",";
        oss << *current;
        first = false;
        current = current->getNext();
    }
    oss << "]";
    return oss.str();
}
template <typename Trait>
// T10: operator>> 
istream& operator>>(istream& is, LinkedList<Trait>& list){
    LinkedList<Trait> temp;
    char token = '\0';
    if(!(is >> ws >> token) || token != '['){
        is.setstate(ios::failbit);
        return is;
    }
    if(is >> ws && is.peek() == ']'){
        is.get();
        list = std::move(temp);
        return is;
    }
    while(true){
        typename LinkedList<Trait>::value_type value{};
        Ref ref{};
        if(!(is >> ws >> token) || token != '('){
            is.setstate(ios::failbit);
            return is;
        }
        if(!(is >> value))
            return is;
        if(!(is >> ws >> token) || token != ','){
            is.setstate(ios::failbit);
            return is;
        }
        if(!(is >> ref))
            return is;
        if(!(is >> ws >> token) || token != ')'){
            is.setstate(ios::failbit);
            return is;
        }
        temp.push_back(value, ref);
        if(!(is >> ws >> token))
            return is;
        if(token == ']')
            break;
        if(token != ','){
            is.setstate(ios::failbit);
            return is;
        }
    }

    list = std::move(temp);
    return is;
}
template <typename Trait>
// T11: operator<< 
ostream& operator<<(ostream& os, const LinkedList<Trait>& list){
    return os << list.toString();
}

#endif // __LINKEDLIST_H__
