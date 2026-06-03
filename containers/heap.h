#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <shared_mutex>
#include <fstream>
#include "vector.h"
#include "../types.h"
#include "traits.h"
#include "../types.h"
using namespace std;

template<typename T>
struct HeapNode {// Nodo del heap
    using value_type = T;
    T   m_data; // valor del nodo
    Ref m_ref;
    HeapNode() : m_data(T{}), m_ref(Ref{}) {}// constructor por defecto
    HeapNode(T data, Ref ref) : m_data(data), m_ref(ref) {}// constructor con parámetros

    friend ostream& operator<<(ostream& os, const HeapNode& n) {// operador de inserción para imprimir el nodo
        return os << "(" << n.m_data << "," << n.m_ref << ")";
    }
};

template<typename Trait>
class Heap {// Clase Heap parametrizada por un trait que define el tipo de nodo, el tipo de valor y el comparador
public:
    using value_type = typename Trait::value_type;// tipo de valor almacenado en el heap
    using Comp       = typename Trait::Comp;// tipo de comparador para mantener el orden del heap
    using Node       = typename Trait::Node;// tipo de nodo del heap       
    using MySelf     = Heap<Trait>;

private:
    Vector<Trait>        m_vec;// vector que almacena los nodos del heap
    mutable shared_mutex m_mtx;// mutex para control de concurrencia
    Comp                 m_comp;// comparador para mantener el orden del heap

    //indices
    size_t parent(size_t i) const { return (i - 1) / 2; }
    size_t right(size_t i)  const { return 2 * i + 2;   }
    size_t left(size_t i)   const { return 2 * i + 1;   }

    //swap
    void swap(size_t i, size_t j) {// intercambia dos nodos en el vector
        std::swap(m_vec[i], m_vec[j]);
    }

    //heapifyUp
    void heapifyUp(size_t i) {// mantiene la propiedad del heap hacia arriba
        while (i > 0 && m_comp(m_vec[i].m_data, m_vec[parent(i)].m_data)) {
            swap(i, parent(i));
            i = parent(i);
        }
    }

    //heapifyDown
    void heapifyDown(size_t i) {// mantiene la propiedad del heap hacia abajo
        size_t n    = m_vec.size();
        size_t best = i;
        size_t l    = left(i);
        size_t r    = right(i);
        if (l < n && m_comp(m_vec[l].m_data, m_vec[best].m_data)) best = l;
        if (r < n && m_comp(m_vec[r].m_data, m_vec[best].m_data)) best = r;
        if (best != i) { swap(i, best); heapifyDown(best); }
    }

    string treeView() const {// convierte el heap a una representación de árbol en forma de string
        size_t n = m_vec.size();
        if (n == 0) return "  (vacio)";
        ostringstream oss;
        size_t level_start = 0;
        size_t level_size  = 1;
        while (level_start < n) {
            oss << "  ";
            for (size_t i = level_start; i < min(level_start + level_size, n); ++i)
                oss << m_vec[i].m_data << " ";
            oss << "\n";
            level_start += level_size;
            level_size  *= 2;
        }
        return oss.str();
    }

public:
    //constructores
    Heap() : m_vec(32) {}// constructor por defecto con capacidad inicial de 32

    //copy constructor
    Heap(const Heap& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_vec = other.m_vec;
    }
    //move constructor
    Heap(Heap&& other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_vec = move(other.m_vec);
    }
    //copy assignment
    Heap& operator=(const Heap& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            shared_lock<shared_mutex> olock(other.m_mtx);
            m_vec = other.m_vec;
        }
        return *this;
    }
    //move assignment
    Heap& operator=(Heap&& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            unique_lock<shared_mutex> olock(other.m_mtx);
            m_vec = move(other.m_vec);
        }
        return *this;
    }
    virtual ~Heap() {}

    //insert
    void insert(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_vec.push_back(Node(value, ref));
        heapifyUp(m_vec.size() - 1);
    }

    //peek
    tuple<value_type, Ref> peek() const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (m_vec.size() == 0) throw runtime_error("heap vacio");
        return make_tuple(m_vec[0].m_data, m_vec[0].m_ref);
    }

    //extrac
    tuple<value_type, Ref> extract() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_vec.size() == 0) throw runtime_error("heap vacio");
        auto result  = make_tuple(m_vec[0].m_data, m_vec[0].m_ref);
        m_vec[0]     = m_vec[m_vec.size() - 1];
        m_vec.pop_back();
        if (m_vec.size() > 0) heapifyDown(0);
        return result;
    }



    bool   isEmpty() const { shared_lock<shared_mutex> lock(m_mtx); return m_vec.size() == 0; }
    size_t size()    const { shared_lock<shared_mutex> lock(m_mtx); return m_vec.size(); }

    //forEach
    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        shared_lock<shared_mutex> lock(m_mtx);
        m_vec.forEach(func, forward<Args>(args)...);
    }

    auto begin() { return m_vec.begin(); }
    auto end()   { return m_vec.end();   }

    //toString
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "Array: [";
        bool first = true;
        const_cast<Vector<Trait>&>(m_vec).forEachNode([&](Node& n) {
            if (!first) oss << ",";
            oss << n;
            first = false;
        });
        oss << "]\nTree:\n" << treeView();
        return oss.str();
    }

    // operator<<
    friend ostream& operator<<(ostream& os, const Heap& h) {
        return os << h.toString();
    }

    //operator>>
    friend istream& operator>>(istream& is, Heap& h) {
        T2 ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type val; Ref ref; T2 comma, paren;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> val >> comma >> ref >> paren)
                    if (comma == ',' && paren == ')')
                        h.insert(val, ref);
        return is;
    }
};

#endif // __HEAP_H__