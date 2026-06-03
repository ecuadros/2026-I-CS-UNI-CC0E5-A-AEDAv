#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <shared_mutex> // shared_mutex
#include "general_iterator.h"
#include "util.h"
#include <mutex>
#include "../types.h"
#include "traits.h"
using namespace std;

template <typename Trait>
class Vector {
public:
    using value_type      = typename Trait::value_type;
    using Node            = typename Trait::Node;
    using Comp            = typename Trait::Comp;
    class ForwardIterator {
        Node* m_ptr;
    public:
        ForwardIterator(Node* p) : m_ptr(p) {}
        Node&            operator*()  { return *m_ptr; }
        ForwardIterator& operator++() { ++m_ptr; return *this; }
        bool operator==(const ForwardIterator& o) const { return m_ptr == o.m_ptr; }
        bool operator!=(const ForwardIterator& o) const { return m_ptr != o.m_ptr; }
    };
    class BackwardIterator {
        Node* m_ptr;
    public:
        BackwardIterator(Node* p) : m_ptr(p) {}
        Node&             operator*()  { return *m_ptr; }
        BackwardIterator& operator++() { --m_ptr; return *this; }
        bool operator==(const BackwardIterator& o) const { return m_ptr == o.m_ptr; }
        bool operator!=(const BackwardIterator& o) const { return m_ptr != o.m_ptr; }
    };

private:
    size_t               m_capacity;
    size_t               m_size;
    Node                *m_data;
    mutable shared_mutex m_mtx;

    void resize() {
        m_capacity = (m_capacity < 10) ? m_capacity + 10 : m_capacity * 2;
        Node* nd = new Node[m_capacity];
        for (size_t i = 0; i < m_size; ++i) nd[i] = m_data[i];
        delete[] m_data;
        m_data = nd;
    }

public:
    Vector(size_t capacity = 10)
        : m_capacity(capacity), m_size(0), m_data(new Node[capacity]) {}

    Vector(const Vector& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_capacity = other.m_capacity;
        m_size     = other.m_size;
        m_data     = new Node[m_capacity];
        for (size_t i = 0; i < m_size; ++i) m_data[i] = other.m_data[i];
    }
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            shared_lock<shared_mutex> olock(other.m_mtx);
            unique_lock<shared_mutex> lock(m_mtx);
            delete[] m_data;
            m_capacity = other.m_capacity;
            m_size     = other.m_size;
            m_data     = new Node[m_capacity];
            for (size_t i = 0; i < m_size; ++i) m_data[i] = other.m_data[i];
        }
        return *this;
    }
    Vector(Vector&& other) : m_capacity(0), m_size(0), m_data(nullptr) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_capacity       = exchange(other.m_capacity, 0);
        m_size           = exchange(other.m_size, 0);
        m_data           = exchange(other.m_data, nullptr);
    }
    Vector& operator=(Vector&& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            unique_lock<shared_mutex> olock(other.m_mtx);
            delete[] m_data;
            m_capacity       = exchange(other.m_capacity, 0);
            m_size           = exchange(other.m_size, 0);
            m_data           = exchange(other.m_data, nullptr);
        }
        return *this;
    }
    virtual ~Vector() { delete[] m_data; }

    //push back
    virtual void push_back(Node node) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == m_capacity) resize();
        m_data[m_size++] = node;
    }

    //pop back 
    virtual void pop_back() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) throw out_of_range("vector vacio");
        --m_size;
    }

    //operator[]
    Node& operator[](size_t index) {
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("indice fuera de rango");
        return m_data[index];
    }
    const Node& operator[](size_t index) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("indice fuera de rango");
        return m_data[index];
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    ForwardIterator  begin() { return ForwardIterator (m_data); }
    ForwardIterator  end()   { return ForwardIterator (m_data + m_size); }
    BackwardIterator rbegin(){ return BackwardIterator(m_data + m_size - 1); }
    BackwardIterator rend()  { return BackwardIterator(m_data - 1); }

    //forEach
    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        for (size_t i = 0; i < m_size; ++i)
            func(m_data[i].m_data, forward<Args>(args)...);
    }

    //forEachNode
    template<typename Func, typename... Args>
    void forEachNode(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        for (size_t i = 0; i < m_size; ++i)
            func(m_data[i], forward<Args>(args)...);
    }

    //toString
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "[";
        for(size_t i = 0; i < m_size; ++i){
            if (i) oss << ",";
            oss << m_data[i];
        }
        oss << "]";
        return oss.str();
    }

    friend ostream& operator<<(ostream& os, const Vector& v) {
        return os << v.toString();
    }
};

void DemoVector();
void DemoConcurrentVector();

#endif // __VECTOR_H__