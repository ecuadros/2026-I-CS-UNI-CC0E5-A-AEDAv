#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include "../types.h"
using namespace std;

template <typename T>
class Vector{
    using value_type = T;
private:
    size_t  m_capacity;
    size_t  m_size;
    T * m_data;
public:
    Vector(size_t capacity);
    virtual ~Vector();
    virtual void push_back(T value);
    virtual T  get(size_t index);
    virtual size_t  size();
    virtual string toString();
};

template <typename T>
Vector<T>::Vector(size_t capacity){
    m_capacity = capacity;
    m_size = 0;
    m_data = new T[capacity];
}

template <typename T>
Vector<T>::~Vector(){
    delete[] m_data;
}

template <typename T>
void Vector<T>::push_back(T value){
    if(m_size < m_capacity)
        m_data[m_size++] = value;
}

template <typename T>
T Vector<T>::get(size_t index){
    if(index >= 0 && index < m_size)
        return m_data[index];
    throw std::out_of_range("Index out of range");
}

template <typename T>
size_t Vector<T>::size(){
    return m_size;
}

template <typename T>
string Vector<T>::toString(){
    if (m_size == 0)
        return "[]";

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
    char c; 
    
    is >> c;
    if (c != '[') {
        is.setstate(ios::failbit); 
        return is;
    }

    if ((is >> ws).peek() == ']')
        return is >> c;
           
    T m_data;
    
    while (is >> m_data) {
        v.push_back(m_data); 
        is >> c; 
        if (c == ']') 
            break; 
        else if (c != ',') {
            is.setstate(ios::failbit); 
            break; 
        }
    }
    
    return is;
}

void DemoVector();

#endif // __VECTOR_H__
