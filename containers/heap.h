#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <tuple>
#include "../types.h"
#include "vector.h"
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

private:
    Vector<Trait> m_vec;
    Comp          m_comp;
    mutable shared_mutex m_mtx;

    static size_t parent(size_t i) { return (i - 1) / 2; }
    static size_t left  (size_t i) { return 2 * i + 1;   }
    static size_t right (size_t i) { return 2 * i + 2;   }

    // Suben/bajan sin tomar m_mtx: corren bajo el lock del metodo publico.
    void heapifyUp(size_t i) {
        while(i > 0 && m_comp(m_vec[i].getDataRef(), m_vec[parent(i)].getDataRef())) {
            swap(m_vec[i], m_vec[parent(i)]);
            i = parent(i);
        }
    }

    void heapifyDown(size_t i) {
        size_t n = m_vec.size();
        while(true) {
            size_t best = i, l = left(i), r = right(i);
            if(l < n && m_comp(m_vec[l].getDataRef(), m_vec[best].getDataRef())) best = l;
            if(r < n && m_comp(m_vec[r].getDataRef(), m_vec[best].getDataRef())) best = r;
            if(best == i) return;
            swap(m_vec[i], m_vec[best]);
            i = best;
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
        if(m_vec.size() == 0) throw runtime_error("Heap::extract: empty");
        tuple<value_type, Ref> top{ m_vec[0].getDataRef(), m_vec[0].getRef() };
        swap(m_vec[0], m_vec[m_vec.size() - 1]);
        m_vec.pop_back();
        if(m_vec.size() > 0) heapifyDown(0);
        return top;
    }

    tuple<value_type, Ref> peek() {
        shared_lock<shared_mutex> lock(m_mtx);
        if(m_vec.size() == 0) throw runtime_error("Heap::peek: empty");
        return { m_vec[0].getDataRef(), m_vec[0].getRef() };
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
        for(size_t i = 0; i < m_vec.size(); ++i) {
            if(i > 0) oss << ",";
            oss << m_vec[i];
        }
        oss << "]";
        return oss.str();
    }

    friend ostream& operator<<(ostream& os, Heap& h) {
        return os << h.toString();
    }

    friend istream& operator>>(istream& is, Heap& h) {
        char ch;
        if(!(is >> ch) || ch != '[') { is.setstate(ios_base::failbit); return is; }
        if((is >> ws).peek() == ']') { is >> ch; return is; }
        VectorNode<value_type> node;
        while(is >> node) {
            h.insert(node.getData(), node.getRef());
            is >> ch;
            if(ch == ']') break;
            else if(ch != ',') { is.setstate(ios_base::failbit); break; }
        }
        return is;
    }
};

#endif // __HEAP_H__
