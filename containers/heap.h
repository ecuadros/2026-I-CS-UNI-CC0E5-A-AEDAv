#ifndef __HEAP_H__
#define __HEAP_H__

#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include "../types.h"
#include "traits.h"
#include "vector.h"
using namespace std;

template <typename T>
struct MinHeapTrait : public BaseTrait<VectorNode<T>, less<T>> {
};

template <typename T>
struct MaxHeapTrait : public BaseTrait<VectorNode<T>, greater<T>> {
};

template<typename Trait>
class Heap {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = Heap<Trait>;

private:
    Vector<Trait>        m_vec;
    Comp                 m_comp;
    mutable shared_mutex m_mtx;

    static size_t parent(size_t index)     { return (index - 1) / 2; }
    static size_t leftChild(size_t index)  { return 2 * index + 1; }
    static size_t rightChild(size_t index) { return 2 * index + 2; }

    void internal_heapifyUp(size_t index) {
        while (index > 0) {
            size_t parentIndex = parent(index);
            if (!m_comp(m_vec[index], m_vec[parentIndex])) {
                break;
            }
            m_vec.swap(index, parentIndex);
            index = parentIndex;
        }
    }

    void internal_heapifyDown(size_t index) {
        size_t count = m_vec.size();
        while (true) {
            size_t best = index;
            size_t left = leftChild(index);
            size_t right = rightChild(index);

            if (left < count && m_comp(m_vec[left], m_vec[best])) {
                best = left;
            }
            if (right < count && m_comp(m_vec[right], m_vec[best])) {
                best = right;
            }
            if (best == index) {
                break;
            }

            m_vec.swap(index, best);
            index = best;
        }
    }

    string treeToString() const {
        if (m_vec.size() == 0) {
            return "(vacio)";
        }

        ostringstream oss;
        size_t levelStart = 0;
        size_t levelSize = 1;
        while (levelStart < m_vec.size()) {
            size_t end = levelStart + levelSize;
            if (end > m_vec.size()) {
                end = m_vec.size();
            }

            oss << "  ";
            for (size_t i = levelStart; i < end; ++i) {
                if (i > levelStart) {
                    oss << " ";
                }
                oss << m_vec[i];
            }
            if (end < m_vec.size()) {
                oss << "\n";
            }

            levelStart += levelSize;
            levelSize *= 2;
        }
        return oss.str();
    }

public:
    Heap(size_t capacity = 10) : m_vec(capacity), m_comp() {}

    Heap(const MySelf &other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_vec = other.m_vec;
        m_comp = other.m_comp;
    }

    Heap(MySelf &&other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_vec = std::move(other.m_vec);
        m_comp = std::move(other.m_comp);
    }

    MySelf& operator=(const MySelf &other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            shared_lock<shared_mutex> otherLock(other.m_mtx);
            m_vec = other.m_vec;
            m_comp = other.m_comp;
        }
        return *this;
    }

    MySelf& operator=(MySelf &&other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            unique_lock<shared_mutex> otherLock(other.m_mtx);
            m_vec = std::move(other.m_vec);
            m_comp = std::move(other.m_comp);
        }
        return *this;
    }

    ~Heap() = default;

    void heapifyUp(size_t index) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (index >= m_vec.size()) {
            throw out_of_range("Indice fuera de rango");
        }
        internal_heapifyUp(index);
    }

    void heapifyDown(size_t index) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (index >= m_vec.size()) {
            throw out_of_range("Indice fuera de rango");
        }
        internal_heapifyDown(index);
    }

    void insert(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_vec.push_back(value, ref);
        internal_heapifyUp(m_vec.size() - 1);
    }

    tuple<value_type, Ref> extract() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_vec.size() == 0) {
            throw runtime_error("Heap vacio");
        }

        auto result = make_tuple(m_vec[0], m_vec.getRef(0));
        if (m_vec.size() == 1) {
            m_vec.pop_back();
            return result;
        }

        m_vec.swap(0, m_vec.size() - 1);
        m_vec.pop_back();
        internal_heapifyDown(0);
        return result;
    }

    tuple<value_type, Ref> peek() const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (m_vec.size() == 0) {
            throw runtime_error("Heap vacio");
        }
        return make_tuple(m_vec[0], m_vec.getRef(0));
    }

    bool isEmpty() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_vec.size() == 0;
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_vec.size();
    }

    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "Array: [";
        for (size_t i = 0; i < m_vec.size(); ++i) {
            if (i > 0) {
                oss << ",";
            }
            oss << "(" << m_vec[i] << "," << m_vec.getRef(i) << ")";
        }
        oss << "]\nTree:\n" << treeToString();
        return oss.str();
    }

    friend ostream& operator<<(ostream &os, const MySelf &heap) {
        return os << heap.toString();
    }

    friend istream& operator>>(istream &is, MySelf &heap) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return is;
        }

        value_type value;
        Ref ref;
        char comma;
        char closeParen;

        while (is >> ch && ch != ']') {
            if (ch == '(' && is >> value >> comma >> ref >> closeParen) {
                if (comma == ',' && closeParen == ')') {
                    heap.insert(value, ref);
                }
            }
        }
        return is;
    }
};

void DemoMinHeap();
void DemoMaxHeap();
void HeapDemo();

#endif // __HEAP_H__
