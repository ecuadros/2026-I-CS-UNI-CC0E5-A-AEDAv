#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include "general_iterator.h"
#include "traits.h"
#include "util.h"
#include "../types.h"
using namespace std;

// Extra 1 nodo base reutilizable
// Permite que LLNode sea nodo simple y que DLLNode reutilice 
template <typename T, typename NodeType>
class LLNodeBase {
protected:
    using Node = NodeType;
private:
    T m_data;
    Ref m_ref;
    Node *m_next;
public:
    LLNodeBase() : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNodeBase(T data, Ref ref, Node *next = nullptr)
        : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNodeBase() {}

    T getData() const { return m_data; }
    T& getDataRef() { return m_data; }
    void setData(T data) { m_data = data; }
    Ref getRef() const { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
    Node* getNext() const { return m_next; }
    Node*& getNextRef() { return m_next; }
    void setNext(Node *next) { m_next = next; }
};

template <typename T>
class LLNode : public LLNodeBase<T, LLNode<T>> {
public:
    using LLNodeBase<T, LLNode<T>>::LLNodeBase;
};

template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>> {
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

template <typename T>
struct AscendingLinkedListTrait {
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait {
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = greater<T>;
};

template <typename Trait>
class LinkedList {
public:
    using value_type = typename Trait::value_type;
    using Node = typename Trait::Node;
    using Comp = typename Trait::Comp;
    using MySelf = LinkedList<Trait>;
    using forward_iterator = LinkedListForwardIterator<MySelf>;
    friend forward_iterator;

protected:
    Node *m_pRoot = nullptr;
    Node *m_tail = nullptr;
    size_t m_size = 0;
    Comp m_comp;
    mutable shared_mutex m_mtx;

    void clear_unlocked() {
        Node *current = m_pRoot;
        while (current) {
            Node *next = current->getNext();
            delete current;
            current = next;
        }
        m_pRoot = nullptr;
        m_tail = nullptr;
        m_size = 0;
    }

    void internal_insert(Node *&pPrev, const value_type &value, Ref ref) {
        if (!pPrev || m_comp(value, pPrev->getDataRef())) {
            pPrev = new Node(value, ref, pPrev);
            ++m_size;
            return;
        }
        internal_insert(pPrev->getNextRef(), value, ref);
    }

    void refresh_tail_unlocked() {
        m_tail = m_pRoot;
        while (m_tail && m_tail->getNext()) {
            m_tail = m_tail->getNext();
        }
    }

public:
    LinkedList() {}

    LinkedList(const LinkedList &other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node *curr = other.m_pRoot; curr; curr = curr->getNext()) {
            push_back(curr->getData(), curr->getRef());
        }
    }

    LinkedList(LinkedList &&other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = exchange(other.m_pRoot, nullptr);
        m_tail = exchange(other.m_tail, nullptr);
        m_size = exchange(other.m_size, 0);
    }

    LinkedList& operator=(const LinkedList &other) {
        if (this == &other) return *this;
        LinkedList temp(other);
        unique_lock<shared_mutex> lock(m_mtx);
        clear_unlocked();
        m_pRoot = exchange(temp.m_pRoot, nullptr);
        m_tail = exchange(temp.m_tail, nullptr);
        m_size = exchange(temp.m_size, 0);
        return *this;
    }

    LinkedList& operator=(LinkedList &&other) {
        if (this == &other) return *this;
        unique_lock<shared_mutex> lock(m_mtx);
        unique_lock<shared_mutex> otherLock(other.m_mtx);
        clear_unlocked();
        m_pRoot = exchange(other.m_pRoot, nullptr);
        m_tail = exchange(other.m_tail, nullptr);
        m_size = exchange(other.m_size, 0);
        return *this;
    }

    virtual ~LinkedList() {
        unique_lock<shared_mutex> lock(m_mtx);
        clear_unlocked();
    }

    // LL Circular insercion 
    
    virtual void insert(const value_type &value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, value, ref);
        refresh_tail_unlocked();
    }

    virtual void push_front(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_pRoot = new Node(value, ref, m_pRoot);
        if (m_size == 0) {
            m_tail = m_pRoot;
        }
        ++m_size;
    }

    virtual void push_back(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (!m_pRoot) {
            m_pRoot = newNode;
            m_tail = newNode;
        } else {
            m_tail->setNext(newNode);
            m_tail = newNode;
        }
        ++m_size;
    }

    virtual tuple<value_type, Ref> pop_front() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("La lista esta vacia");
        Node *temp = m_pRoot;
        auto result = make_tuple(temp->getData(), temp->getRef());
        m_pRoot = temp->getNext();
        delete temp;
        --m_size;
        if (m_size == 0) m_tail = nullptr;
        return result;
    }

    virtual tuple<value_type, Ref> pop_back() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("La lista esta vacia");
        auto result = make_tuple(m_tail->getData(), m_tail->getRef());
        if (m_pRoot == m_tail) {
            delete m_pRoot;
            m_pRoot = nullptr;
            m_tail = nullptr;
        } else {
            Node *prev = m_pRoot;
            while (prev->getNext() != m_tail) {
                prev = prev->getNext();
            }
            delete m_tail;
            m_tail = prev;
            m_tail->setNext(nullptr);
        }
        --m_size;
        return result;
    }

    virtual value_type& operator[](size_t index) {
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("Indice fuera de rango");
        Node *act = m_pRoot;
        for (size_t i = 0; i < index; ++i) {
            act = act->getNext();
        }
        return act->getDataRef();
    }

    virtual size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end() { return forward_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        for (auto &item : *this) {
            func(item, forward<Args>(args)...);
        }
    }

    friend ostream& operator<<(ostream& os, const LinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node *act = list.m_pRoot;
        while (act) {
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (act->getNext()) os << ",";
            act = act->getNext();
        }
        os << "]";
        return os;
    }

    friend istream& operator>>(istream& is, LinkedList& list) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.setstate(ios_base::failbit);
            return is;
        }
        while (is >> ch && ch != ']') {
            if (ch != '(') continue;
            value_type val;
            Ref ref;
            char comma;
            char parenClose;
            if (is >> val >> comma >> ref >> parenClose && comma == ',' && parenClose == ')') {
                list.push_back(val, ref);
            } else {
                is.setstate(ios_base::failbit);
                return is;
            }
        }
        return is;
    }
};

#endif // __LINKEDLIST_H__
