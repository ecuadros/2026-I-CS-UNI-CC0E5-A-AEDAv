#ifndef __STACK_H__
#define __STACK_H__
#include "vector.h"
using namespace std;

template<typename T>
class Stack {
    Vector<T> m_data;
public:
    Stack() {}
    Stack(const Stack& other)            : m_data(other.m_data)        {}
    Stack(Stack&& other) noexcept        : m_data(move(other.m_data))  {}
    Stack& operator=(const Stack& other) { m_data = other.m_data;        return *this; }
    Stack& operator=(Stack&& other)      { m_data = move(other.m_data);  return *this; }

    void   push(T val)          { m_data.push_back(val, Ref()); }
    T      operator[](size_t i) const { return m_data.get(i); }
    size_t size()  const        { return m_data.size(); }
    bool   empty() const        { return m_data.size() == 0; }
};

#endif // __STACK_H__
