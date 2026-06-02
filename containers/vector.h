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
using namespace std;

template <typename T1, typename T2>
ostream& operator<<(ostream& os, const std::tuple<T1, T2>& t) {
    return os << "{" << std::get<0>(t) << ":" << std::get<1>(t) << "}";
}

template <typename Container>
class vector_forward_iterator : public general_iterator<Container, vector_forward_iterator<Container>> {
public:
    using MySelf = vector_forward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() { this->m_pNode++; return *this; }
};

template <typename Container>
class vector_backward_iterator : public general_iterator<Container, vector_backward_iterator<Container>> {
public:
    using MySelf = vector_backward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() { this->m_pNode--; return *this; }
};

template <typename T>
class VectorNode{
public:
    using value_type = T;

private:
    T   m_data;
    Ref m_ref;

public:
    VectorNode() : m_data(T()), m_ref(Ref()) {}
    VectorNode(T data, Ref ref) : m_data(data), m_ref(ref) {}
    VectorNode(const VectorNode &other) : m_data(other.m_data), m_ref(other.m_ref) {}
    VectorNode(VectorNode &&other) : m_data(move(other.m_data)), m_ref(move(other.m_ref)) {}
    VectorNode& operator=(const VectorNode &other) {
        m_data = other.m_data;
        m_ref = other.m_ref;
        return *this;
    }
    VectorNode& operator=(VectorNode &&other) {
        m_data = move(other.m_data);
        m_ref = move(other.m_ref);
        return *this;
    }

    T    getData() const { return m_data; }
    T&   getDataRef() { return m_data; }
    void setData(T data) { m_data = data; }
    Ref  getRef() { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
};

template <typename T>
ostream& operator<<(ostream& os, VectorNode<T>& node){
    return os << "(" << node.getData() << ", " << node.getRef() << ")";
}

template <typename T>
class Vector{
public:
    using  value_type = T;
    using  forward_iterator   = vector_forward_iterator < Vector<T> > ;
    friend forward_iterator;
    using  backward_iterator  = vector_backward_iterator< Vector<T> > ;
    friend backward_iterator;
    using  Node               = VectorNode<T>;
private:
    size_t  m_capacity;
    size_t  m_size;
    Node   *m_data;
    mutable shared_mutex m_mtx;
    void    resize();
public:
    Vector(size_t capacity = 10);
    virtual ~Vector();
    virtual void push_back(value_type value, Ref ref);
    virtual Node getNode(size_t index) const;
    virtual void swapNodes(size_t i, size_t j);
    virtual void pop_back();
    virtual size_t size() const;
    virtual string toString() const;

    forward_iterator begin() { return forward_iterator(this, m_data); }
    forward_iterator end()   { return forward_iterator(this, m_data + m_size); }

    backward_iterator rbegin() { return backward_iterator(this, m_data + m_size - 1); }
    backward_iterator rend()   { return backward_iterator(this, m_data - 1); }
    
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        unique_lock<shared_mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...  args){
        unique_lock<shared_mutex> lock(m_mtx);
        if(m_size == 0) return;
        ::ForEach(rbegin(), rend(), func, std::forward<Args>(args)... );
    }

    Vector(const Vector& other) : m_capacity(other.m_capacity), m_size(other.m_size) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_data = new Node[m_capacity];
        for (size_t i = 0; i < m_size; ++i) m_data[i] = other.m_data[i];
    }
    
    Vector(Vector&& other) noexcept : m_capacity(0), m_size(0), m_data(nullptr) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_size = std::exchange(other.m_size, 0);
        m_data = std::exchange(other.m_data, nullptr);
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            shared_lock<shared_mutex> olock(other.m_mtx);
            delete[] m_data;
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            m_data = new Node[m_capacity];
            for(size_t i = 0; i < m_size; ++i) m_data[i] = other.m_data[i];
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            unique_lock<shared_mutex> olock(other.m_mtx);
            delete[] m_data;
            m_capacity = std::exchange(other.m_capacity, 0);
            m_size = std::exchange(other.m_size, 0);
            m_data = std::exchange(other.m_data, nullptr);
        }
        return *this;
    }

    value_type& operator[](size_t index) {
        shared_lock<shared_mutex> lock(m_mtx);
        if(index >= m_size) throw out_of_range("Indice fuera de rango");
        return m_data[index].getDataRef();
    }
    
    value_type operator[](size_t index) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if(index >= m_size) throw out_of_range("Indice fuera de rango");
        return m_data[index].getData();
    }

};

template <typename T>
Vector<T>::Vector(size_t capacity){
    m_capacity = capacity;
    m_size = 0;
    m_data = new Node[capacity];
}

template <typename T>
Vector<T>::~Vector(){
    delete [] m_data;
}

template <typename T>
void Vector<T>::resize(){
    m_capacity = (m_capacity < 10) ? m_capacity+10 : m_capacity * 2;
    Node * new_data = new Node[m_capacity];
    for(size_t i = 0; i < m_size; ++i)
        new_data[i] = m_data[i];
    delete [] m_data;
    m_data = new_data;
}

template <typename T>
void Vector<T>::push_back(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    if(m_size == m_capacity) resize();
    m_data[m_size++] = Node(value, ref);
}

template <typename T>
size_t Vector<T>::size() const{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size;
}

template <typename T>
string Vector<T>::toString() const{
    shared_lock<shared_mutex> lock(m_mtx);
    ostringstream oss;
    oss << "[";
    for(size_t i = 0; i < m_size; ++i){
        if(i > 0) oss << ",";
        oss << m_data[i];
    }
    oss << "]";
    return oss.str();
}

template <typename T>
typename Vector<T>::Node Vector<T>::getNode(size_t index) const {
    shared_lock<shared_mutex> lock(m_mtx);
    if (index >= m_size) throw out_of_range("Indice fuera de rango");
    return m_data[index]; 
}

template <typename T>
void Vector<T>::swapNodes(size_t i, size_t j) {
    unique_lock<shared_mutex> lock(m_mtx);
    if (i >= m_size || j >= m_size) throw out_of_range("Indice fuera de rango");
    std::swap(m_data[i], m_data[j]);
}

template <typename T>
void Vector<T>::pop_back() {
    unique_lock<shared_mutex> lock(m_mtx);
    if (m_size > 0) m_size--;
}

// ---------------------------------------------

template <typename T>
ostream& operator<<(ostream& os, const Vector<T>& v){
    return os << v.toString();
}

template <typename T>
istream& operator>>(istream& is, Vector<T>& v){
    return is;
}

void DemoVector();
void DemoConcurrentVector();

#endif // __VECTOR_H__