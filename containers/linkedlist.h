#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <cstddef>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../types.h"
#include "general_iterator.h"
#include "traits.h"
#include "util.h"

using namespace std;

template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
public:
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++(){
        if(this->m_pNode){
            this->m_pNode = this->m_pNode->getNext();
        }
        return *this;
    }
};

// Reutilizacion Node en CLL
template <typename T, typename NodeType = void>
class LLNode{
public:
    using value_type = T;
    using Node = std::conditional_t<std::is_void_v<NodeType>, LLNode<T>, NodeType>;

private:
    T m_data;
    Ref m_ref;
    Node *m_next;

public:
    LLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data, Ref ref) : m_data(data), m_ref(ref), m_next(nullptr) {}
    LLNode(T data, Ref ref, Node *next) : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T getData() const { return m_data; }
    T &getDataRef() { return m_data; }
    const T &getDataRef() const { return m_data; }
    void setData(T data) { m_data = data; }
    Ref getRef() const { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
    Node *getNext() const { return m_next; }
    Node *&getNextRef() { return m_next; }
    void setNext(Node *next) { m_next = next; }
};

template <typename T, typename NodeType = void>
using LLNodeBase = LLNode<T, NodeType>;

// Reutilizacion operador <=
template <typename T, typename NodeType>
bool operator<(const LLNode<T, NodeType> &lhs, const LLNode<T, NodeType> &rhs){
    return lhs.getData() < rhs.getData();
}

template <typename T, typename NodeType>
bool operator<=(const LLNode<T, NodeType> &lhs, const LLNode<T, NodeType> &rhs){
    return !(rhs < lhs);
}

// Reutilizacion operador >
template <typename T, typename NodeType>
bool operator>(const LLNode<T, NodeType> &lhs, const LLNode<T, NodeType> &rhs){
    return rhs < lhs;
}

template <typename T, typename NodeType>
bool operator>=(const LLNode<T, NodeType> &lhs, const LLNode<T, NodeType> &rhs){
    return !(lhs < rhs);
}

template <typename T>
struct AscendingLinkedListTrait : public BaseTrait<LLNode<T>, std::less<T>>{
};

template <typename T>
struct DescendingLinkedListTrait : public BaseTrait<LLNode<T>, std::greater<T>>{
};

template <typename Trait>
class LinkedList{
public:
    using value_type = typename Trait::value_type;
    using Node = typename Trait::Node;
    using Comp = typename Trait::Comp;
    using MySelf = LinkedList<Trait>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    friend forward_iterator;

protected:
    Node *m_pRoot;
    Node *m_tail;
    size_t m_size;
    Comp m_comp;
    mutable shared_mutex m_mtx;

    void clear_unlocked(){
        Node *current = m_pRoot;
        while(current){
            Node *next = current->getNext();
            delete current;
            current = next;
        }
        m_pRoot = nullptr;
        m_tail = nullptr;
        m_size = 0;
    }

    void rebuild_tail_unlocked(){
        m_tail = m_pRoot;
        while(m_tail && m_tail->getNext()){
            m_tail = m_tail->getNext();
        }
    }

    void internal_insert(Node *&pPrev, const value_type &value, Ref ref){
        if(!pPrev || m_comp(value, pPrev->getData())){
            pPrev = new Node(value, ref, pPrev);
            ++m_size;
            if(!m_tail || pPrev->getNext() == nullptr){
                m_tail = pPrev;
            }
            return;
        }
        internal_insert(pPrev->getNextRef(), value, ref);
    }

    void push_front_unlocked(value_type value, Ref ref){
        m_pRoot = new Node(value, ref, m_pRoot);
        if(m_size == 0){
            m_tail = m_pRoot;
        }
        ++m_size;
    }

    tuple<value_type, Ref> pop_front_unlocked(){
        if(!m_pRoot){
            throw runtime_error("La lista esta vacia");
        }

        Node *temp = m_pRoot;
        auto result = make_tuple(temp->getData(), temp->getRef());
        m_pRoot = m_pRoot->getNext();
        delete temp;
        --m_size;

        if(m_size == 0){
            m_tail = nullptr;
        }
        return result;
    }

    void push_back_unlocked(value_type value, Ref ref){
        Node *newNode = new Node(value, ref);
        if(m_size == 0){
            m_pRoot = newNode;
            m_tail = newNode;
        }else{
            m_tail->setNext(newNode);
            m_tail = newNode;
        }
        ++m_size;
    }

    tuple<value_type, Ref> pop_back_unlocked(){
        if(!m_pRoot){
            throw runtime_error("La lista esta vacia");
        }

        tuple<value_type, Ref> result;
        if(m_pRoot == m_tail){
            result = make_tuple(m_pRoot->getData(), m_pRoot->getRef());
            delete m_pRoot;
            m_pRoot = nullptr;
            m_tail = nullptr;
        }else{
            Node *act = m_pRoot;
            while(act->getNext() != m_tail){
                act = act->getNext();
            }
            result = make_tuple(m_tail->getData(), m_tail->getRef());
            delete m_tail;
            m_tail = act;
            m_tail->setNext(nullptr);
        }
        --m_size;
        return result;
    }

    void insert_unlocked(const value_type &value, Ref ref){
        internal_insert(m_pRoot, value, ref);
        rebuild_tail_unlocked();
    }

public:
    LinkedList() : m_pRoot(nullptr), m_tail(nullptr), m_size(0), m_comp(Comp()) {}

    // Constructor copia
    LinkedList(const LinkedList &other) : m_pRoot(nullptr), m_tail(nullptr), m_size(0), m_comp(other.m_comp) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for(Node *curr = other.m_pRoot; curr != nullptr; curr = curr->getNext()){
            push_back_unlocked(curr->getData(), curr->getRef());
        }
    }

    // Constructor move
    LinkedList(LinkedList &&other) : m_pRoot(nullptr), m_tail(nullptr), m_size(0), m_comp(Comp()) {
        unique_lock<shared_mutex> lockOther(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_tail = std::exchange(other.m_tail, nullptr);
        m_size = std::exchange(other.m_size, 0);
        m_comp = std::move(other.m_comp);
    }

    LinkedList &operator=(const LinkedList &other){
        if(this != &other){
            Node *newRoot = nullptr;
            Node *newTail = nullptr;
            size_t newSize = 0;
            Comp newComp = Comp();

            {
                shared_lock<shared_mutex> lock(other.m_mtx);
                newComp = other.m_comp;
                for(Node *curr = other.m_pRoot; curr != nullptr; curr = curr->getNext()){
                    Node *newNode = new Node(curr->getData(), curr->getRef());
                    if(!newRoot){
                        newRoot = newTail = newNode;
                    }else{
                        newTail->setNext(newNode);
                        newTail = newNode;
                    }
                    ++newSize;
                }
            }

            unique_lock<shared_mutex> lock(m_mtx);
            clear_unlocked();
            m_pRoot = newRoot;
            m_tail = newTail;
            m_size = newSize;
            m_comp = std::move(newComp);
        }
        return *this;
    }

    LinkedList &operator=(LinkedList &&other){
        if(this != &other){
            unique_lock<shared_mutex> lockThis(m_mtx, defer_lock);
            unique_lock<shared_mutex> lockOther(other.m_mtx, defer_lock);
            std::lock(lockThis, lockOther);
            clear_unlocked();
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_tail = std::exchange(other.m_tail, nullptr);
            m_size = std::exchange(other.m_size, 0);
            m_comp = std::move(other.m_comp);
        }
        return *this;
    }

    // Destructor Seguro
    virtual ~LinkedList(){
        unique_lock<shared_mutex> lock(m_mtx);
        clear_unlocked();
    }

    virtual void push_front(value_type value, Ref ref){
        unique_lock<shared_mutex> lock(m_mtx);
        push_front_unlocked(value, ref);
    }

    virtual tuple<value_type, Ref> pop_front(){
        unique_lock<shared_mutex> lock(m_mtx);
        return pop_front_unlocked();
    }

    virtual void push_back(value_type value, Ref ref){
        unique_lock<shared_mutex> lock(m_mtx);
        push_back_unlocked(value, ref);
    }

    virtual tuple<value_type, Ref> pop_back(){
        unique_lock<shared_mutex> lock(m_mtx);
        return pop_back_unlocked();
    }

    virtual void insert(const value_type &value, Ref ref){
        unique_lock<shared_mutex> lock(m_mtx);
        insert_unlocked(value, ref);
    }

    virtual value_type &operator[](size_t index){
        shared_lock<shared_mutex> lock(m_mtx);
        if(index >= m_size){
            throw out_of_range("Indice fuera de rango");
        }
        Node *act = m_pRoot;
        for(size_t i = 0; i < index; ++i){
            act = act->getNext();
        }
        return act->getDataRef();
    }

    virtual size_t size() const{
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    bool empty() const{
        return size() == 0;
    }

    forward_iterator begin(){ return forward_iterator(this, m_pRoot); }
    forward_iterator end(){ return forward_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args){
        shared_lock<shared_mutex> lock(m_mtx);
        for(auto &item : *this){
            func(item, std::forward<Args>(args)...);
        }
    }

    friend ostream &operator<<(ostream &os, const LinkedList &list){
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node *act = list.m_pRoot;
        while(act){
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if(act->getNext()){
                os << ",";
            }
            act = act->getNext();
        }
        os << "]";
        return os;
    }

    friend istream &operator>>(istream &is, LinkedList &list){
        char ch;
        if(!(is >> ch) || ch != '['){
            is.clear(ios_base::failbit);
            return is;
        }

        LinkedList temp;
        value_type val;
        Ref ref;
        char comma, parenClose;
        while(is >> ch && ch != ']'){
            if(ch == '('){
                if(is >> val >> comma >> ref >> parenClose){
                    if(comma == ',' && parenClose == ')'){
                        temp.push_back_unlocked(val, ref);
                    }
                }
            }
        }
        list = std::move(temp);
        return is;
    }
};

#endif // __LINKEDLIST_H__
