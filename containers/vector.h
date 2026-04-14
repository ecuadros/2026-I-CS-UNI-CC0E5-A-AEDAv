#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <mutex>   // mutex
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
using namespace std;

template <typename Container>
class vector_forward_iterator : public general_iterator<Container, vector_forward_iterator<Container>> {
public:
    using general_iterator<Container, vector_forward_iterator<Container>>::general_iterator;
    vector_forward_iterator operator++() { this->m_pNode++; return *this; }
};

template <typename Container>
class vector_backward_iterator : public general_iterator<Container, vector_backward_iterator<Container>> {
public:
    using general_iterator<Container, vector_backward_iterator<Container>>::general_iterator;
    vector_backward_iterator operator++() { this->m_pNode--; return *this; }
};

template <typename T>
class VectorNode{
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
    mutex   m_mtx;
    void    resize();
public:
    Vector(size_t capacity = 10);
    virtual ~Vector();
    virtual void push_back(value_type value, Ref ref);
    virtual value_type  get(size_t index);
    virtual size_t size();
    virtual string toString();

    forward_iterator begin() { return forward_iterator(this, m_data); }
    forward_iterator end()   { return forward_iterator(this, m_data + m_size); }

    backward_iterator rbegin() { return backward_iterator(this, m_data + m_size - 1); }
    backward_iterator rend()   { return backward_iterator(this, m_data - 1); }
    
    // TODO: Agregar control concurrente
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        scoped_lock lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }

    // TODO: Agregar control concurrente
    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...  args){
        scoped_lock lock(m_mtx);
        ::ForEach(rbegin(), rend(), func, std::forward<Args>(args)... );
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
    scoped_lock lock(m_mtx);
    if(m_size == m_capacity) // Overflow
        resize();
    m_data[m_size++] = Node(value, ref);
}

template <typename T>
typename Vector<T>::value_type
Vector<T>::get(size_t index){
    if(index >= 0 && index < m_size)
        return m_data[index].getData();
    throw std::out_of_range("Index out of range");
}

template <typename T>
size_t Vector<T>::size(){
    return m_size;
}

template <typename T>
string Vector<T>::toString(){
    ostringstream oss;
    oss << "[";
    for(size_t i = 0; i < m_size - 1; ++i)
        oss << m_data[i] << ",";
    if(m_size > 0)
        oss << m_data[m_size - 1];
    oss << "]";
    return oss.str();
}

template <typename T>
ostream& operator<<(ostream& os, Vector<T>& v){
    return os << v.toString();
}

// TODO: Implementar como PR
template <typename T>
istream& operator>>(istream& is, Vector<T>& v){
    return is;
}

// template <typename T>
// template <typename Func, typename... Args>
// void Vector<T>::ForEach(Func func, Args &&...  args){
//     ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
// }

void DemoVector();
void DemoConcurrentVector();

#endif // __VECTOR_H__
