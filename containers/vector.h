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
    string input;
    is >> input;
    if(input.empty() || input.front() != '[' || input.back() != ']')
        throw std::invalid_argument("Invalid input format");
    input = input.substr(1, input.size() - 2);

    istringstream iss(input);
    string str;

    while(getline(iss, str, ',')){
        istringstream eStream(str);
        T valor;
        if(eStream >> valor) {
            string r;
            if(eStream >> r) { // 1 A 2
                throw std::invalid_argument("Invalid input format for an element");
            }
            v.push_back(valor);
        } else {
            throw std::invalid_argument("Error parsing the element");
        }
    }
    return is;
}

void DemoVector();

#endif // __VECTOR_H__
