#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <tuple>
#include <limits>
#include <functional>
#include "vector.h"
#include "../types.h"
using namespace std;

template <typename T>
struct MinHeapTrait {
    using value_type = T;
    using Comp       = less<T>;
};

template <typename T>
struct MaxHeapTrait {
    using value_type = T;
    using Comp       = greater<T>;
};

template<typename Trait>
class Heap{
public:
    using value_type = typename Trait::value_type;
    using Comp       = typename Trait::Comp;
    using MySelf     = Heap<Trait>;
    using Node       = typename Vector<Trait>::Node;

private:
    Vector<Trait>        m_vec;
    Comp                 m_comp;
    mutable shared_mutex m_mtx;

    static size_t parentOf  (size_t i) { return (i - 1) / 2; }
    static size_t leftChild (size_t i) { return 2 * i + 1; }
    static size_t rightChild(size_t i) { return 2 * i + 2; }

    void swapNodes(size_t a, size_t b) {
        Node& first  = m_vec[a];
        Node& second = m_vec[b];
        value_type data = first.getData();
        Ref        ref  = first.getRef();
        first.setData(second.getData());  first.setRef(second.getRef());
        second.setData(data);             second.setRef(ref);
    }

    // Sube el elemento hasta que su padre tenga mayor prioridad
    void heapifyUp(size_t index) {
        while (index > 0) {
            size_t parent = parentOf(index);
            if (!m_comp(m_vec[index].getData(), m_vec[parent].getData()))
                break;
            swapNodes(index, parent);
            index = parent;
        }
    }

    // Baja el elemento intercambiandolo con el hijo de mayor prioridad
    void heapifyDown(size_t index) {
        size_t total = m_vec.size();
        while (true) {
            size_t left   = leftChild(index);
            size_t right  = rightChild(index);
            size_t target = index;
            if (left  < total && m_comp(m_vec[left ].getData(), m_vec[target].getData()))
                target = left;
            if (right < total && m_comp(m_vec[right].getData(), m_vec[target].getData()))
                target = right;
            if (target == index)
                break;
            swapNodes(index, target);
            index = target;
        }
    }

public:
    Heap() : m_vec(), m_comp() {}
    ~Heap() {}

    void insert(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_vec.push_back(value, ref);
        heapifyUp(m_vec.size() - 1);
    }

    tuple<value_type, Ref> extract() {
        unique_lock<shared_mutex> lock(m_mtx);
        size_t total = m_vec.size();
        if (total == 0)
            throw out_of_range("Heap::extract sobre heap vacio");
        auto top = make_tuple(m_vec[0].getData(), m_vec[0].getRef());
        swapNodes(0, total - 1);
        m_vec.pop_back();
        if (m_vec.size() > 0)
            heapifyDown(0);
        return top;
    }

    tuple<value_type, Ref> peek() {
        shared_lock<shared_mutex> lock(m_mtx);
        if (m_vec.size() == 0)
            throw out_of_range("Heap::peek sobre heap vacio");
        return make_tuple(m_vec[0].getData(), m_vec[0].getRef());
    }

    bool isEmpty() {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_vec.size() == 0;
    }

    size_t size() {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_vec.size();
    }

    string toString() {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "[";
        size_t total = m_vec.size();
        for (size_t i = 0; i < total; ++i) {
            if (i > 0) oss << ",";
            oss << "(" << m_vec[i].getData() << "," << m_vec[i].getRef() << ")";
        }
        oss << "]";
        return oss.str();
    }

    friend ostream& operator<<(ostream& os, Heap& heap) {
        return os << heap.toString();
    }

    friend istream& operator>>(istream& is, Heap& heap) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.setstate(ios::failbit); return is; }
        value_type value; Ref ref; char comma, paren;
        while (is >> ch && ch != ']')
            if (ch == '(' && (is >> value >> comma >> ref >> paren) && comma == ',' && paren == ')')
                heap.insert(value, ref);
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }
};

#endif // __HEAP_H__
