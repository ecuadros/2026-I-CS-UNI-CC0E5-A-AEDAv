#ifndef __STACK_H__
#define __STACK_H__

#include <stdexcept>
#include <shared_mutex>
#include "vector.h"
using namespace std;

//stack
template<typename T>
class Stack {
    Vector<T>            m_vec;
    mutable shared_mutex m_mtx;
public:
    Stack(size_t capacity = 64) : m_vec(capacity) {}

    //copy constructor
    Stack(const Stack& other):m_vec(other.m_vec.size() + 64){
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (size_t i = 0; i < other.m_vec.size(); ++i)
            m_vec.push_back(other.m_vec[i], 0);
    }
    Stack& operator=(const Stack& other){
        if (this != &other){
            unique_lock<shared_mutex> lock(m_mtx);
            shared_lock<shared_mutex> olock(other.m_mtx);
            m_vec = Vector<T>(other.m_vec.size() + 64);
            for (size_t i = 0; i < other.m_vec.size(); ++i)
                m_vec.push_back(other.m_vec[i], 0);
        }
        return *this;
    }

    void push(T val){
        unique_lock<shared_mutex> lock(m_mtx);
        m_vec.push_back(val, 0);
    }

    void pop(){
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_vec.size() == 0) throw out_of_range("stack vacio");
        m_vec.pop_back();
    }

    T top() const{
        shared_lock<shared_mutex> lock(m_mtx);
        if (m_vec.size() == 0) throw out_of_range("stack vacio");
        return m_vec[m_vec.size() - 1];
    }

    T operator[](size_t i) const{
        shared_lock<shared_mutex> lock(m_mtx);
        return m_vec[i];
    }

    bool empty() const{shared_lock<shared_mutex> lock(m_mtx); return m_vec.size() == 0;}
    size_t size() const{shared_lock<shared_mutex> lock(m_mtx); return m_vec.size();}
};

#endif // __STACK_H__