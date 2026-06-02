#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <stdexcept>
#include <shared_mutex>
#include <utility>
#include <tuple>
#include "vector.h"
#include "util.h"
#include "../types.h"
#include "traits.h"
using namespace std;

template <typename T>
using HeapNode = VectorNode<T>;

// TODO 1: Adaptacion de Traits
template <typename T>
struct MinHeapTrait : public BaseTrait<HeapNode<T>, less<T>> {};

template <typename T>
struct MaxHeapTrait : public BaseTrait<HeapNode<T>, greater<T>> {};

template<typename Trait>
class Heap {
public:
    using value_type = typename Trait::value_type;
    using Comp       = typename Trait::Comp;
    using Node       = typename Trait::Node; // Se mapeará a VectorNode<value_type>
    using MySelf     = Heap<Trait>;
    
private:
    Vector<value_type> m_vec;
    Comp               m_comp;
    mutable shared_mutex m_mtx;

    size_t parent(size_t index) const { return (index - 1) / 2; }
    size_t left(size_t index)   const { return 2 * index + 1; }
    size_t right(size_t index)  const { return 2 * index + 2; }

public:
    // Constructor
    Heap() : m_vec(), m_comp() {}
    Heap(const Vector<value_type>& vec) : m_vec(vec), m_comp() {
        if (m_vec.size() > 0) {
            for (size_t i = parent(m_vec.size() - 1); i > 0; --i) {
                heapifyDown(i - 1);
            }
        }
    }

    // Copy constructor
    Heap(const Heap& other) : m_vec(), m_comp() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_vec = other.m_vec;
        this->m_comp = other.m_comp;
    }

    // Move constructor
    Heap(Heap&& other) noexcept : m_vec(), m_comp() {
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_vec = move(other.m_vec);
        this->m_comp = move(other.m_comp);
    }

    // Copy assignment
    Heap& operator=(const Heap& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            shared_lock<shared_mutex> olock(other.m_mtx);
            this->m_vec = other.m_vec;
            this->m_comp = other.m_comp;
        }
        return *this;
    }

    // Move assignment
    Heap& operator=(Heap&& other) noexcept {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            unique_lock<shared_mutex> olock(other.m_mtx);
            this->m_vec = move(other.m_vec);
            this->m_comp = move(other.m_comp);
        }
        return *this;
    }

    // Destructor
    ~Heap() {}

    // TODO 3: heapifyUp
    void heapifyUp(size_t index) {
        if (index == 0) return;
        size_t i_parent = parent(index);
        
        // Si el actual cumple la condición del Trait contra su padre, "flota" hacia arriba
        if (m_comp(m_vec[index], m_vec[i_parent])) {
            m_vec.swapNodes(index, i_parent);
            heapifyUp(i_parent);
        }
    }

    // TODO 5: heapifyDown
    void heapifyDown(size_t index) {
        size_t i_left = left(index);
        size_t i_right = right(index);
        size_t optimal = index;
        size_t current_size = m_vec.size();

        // Evalua el hijo izquierdo
        if (i_left < current_size && m_comp(m_vec[i_left], m_vec[optimal]))
            optimal = i_left;
        
        // Evalua el hijo derecho
        if (i_right < current_size && m_comp(m_vec[i_right], m_vec[optimal]))
            optimal = i_right;

        // Si la raíz no es la óptima, "se hunde"
        if (optimal != index) {
            m_vec.swapNodes(index, optimal);
            heapifyDown(optimal);
        }
    }

    // TODO 2: Insert
    void insert(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_vec.push_back(value, ref);
        heapifyUp(m_vec.size() - 1);
    }
    
    // TODO 4: Extract
    tuple<value_type, Ref> extract() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_vec.size() == 0) throw out_of_range("Heap vacio");
        
        auto rootNode = m_vec.getNode(0);
        tuple<value_type, Ref> result = make_tuple(rootNode.getData(), rootNode.getRef());

        m_vec.swapNodes(0, m_vec.size() - 1);
        m_vec.pop_back(); // Elimina el último elemento físico

        if (m_vec.size() > 0) {
            heapifyDown(0); // Restaura las propiedades del Heap
        }
        return result;
    } 

    // TODO 6: Peek
    tuple<value_type, Ref> peek() const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (m_vec.size() == 0) throw out_of_range("Heap vacio");
        auto rootNode = m_vec.getNode(0);
        return make_tuple(rootNode.getData(), rootNode.getRef());
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
        return m_vec.toString();
    }

    // TODO 8: operator<<
    friend ostream& operator<<(ostream& os, const Heap& h) {
        os << h.toString();
        return os;
    }

    // TODO 9: operator>>
    friend istream& operator>>(istream& is, Heap& h) {
        Token c;
        if (!(is >> c) || c != '[') { is.clear(ios_base::failbit); return is; }
        value_type val; Ref ref; Token comma, paren;
        while (is >> c && c != ']') {
            if (c == '(') {
                if (is >> val >> comma >> ref >> paren) {
                    if (comma == ',' && paren == ')') {
                        h.insert(val, ref);
                    }
                }
            }
        }
        return is;
    }
};

#endif // __HEAP_H__