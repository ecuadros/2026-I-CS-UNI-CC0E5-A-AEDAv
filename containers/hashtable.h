#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <iostream>
#include <sstream>
#include <tuple>
#include <utility>
#include <limits>
#include <mutex>
#include <shared_mutex>

#include "AVL.h"
#include "../types.h"
using namespace std;

// Iterador clave-valor sobre el recorrido inorder del arbol.
// operator* devuelve (clave, valor&) para usar: for (auto [clave, valor] : tabla)
template <typename Trait>
class ht_kv_iterator {
public:
    using value_type    = typename Trait::value_type;
    using Node          = typename Trait::Node;
    using TreeBase      = BinaryTree<Trait>;
    using base_iterator = typename TreeBase::forward_iterator;

private:
    base_iterator m_it;

public:
    ht_kv_iterator(base_iterator it) : m_it(it) {}

    bool operator==(const ht_kv_iterator& other) const { return m_it == other.m_it; }
    bool operator!=(const ht_kv_iterator& other) const { return !(m_it == other.m_it); }

    ht_kv_iterator& operator++() { ++m_it; return *this; }

    tuple<value_type, Ref&> operator*() {
        Node* node = m_it.getNode();
        return tuple<value_type, Ref&>(node->m_data, node->m_ref);
    }
};

// Tabla tipo mapa construida sobre el AVL: la clave es el dato y el valor es la referencia.
template <typename Trait>
class HashTable : public AVL<Trait> {
public:
    using Base       = AVL<Trait>;
    using TreeBase   = BinaryTree<Trait>;
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using MySelf     = HashTable<Trait>;
    using iterator   = ht_kv_iterator<Trait>;

    HashTable() : Base() {}
    HashTable(const HashTable& other) : Base(other) {}
    HashTable(HashTable&& other) noexcept : Base(move(other)) {}
    HashTable& operator=(const HashTable& other) { Base::operator=(other); return *this; }
    HashTable& operator=(HashTable&& other) noexcept { Base::operator=(move(other)); return *this; }

    // Si la clave existe devuelve su valor; si no, la crea (insert balanceado) y lo devuelve.
    Ref& operator[](const value_type& key) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* node = this->find_node(key);
        if (!node) {
            this->internal_insert(this->m_pRoot, key, Ref());
            node = this->find_node(key);
        }
        return node->m_ref;
    }

    iterator begin() { return iterator(TreeBase::begin()); }
    iterator end()   { return iterator(TreeBase::end()); }

    friend ostream& operator<<(ostream& os, HashTable& table) {
        shared_lock<shared_mutex> lock(table.m_mtx);
        os << "[";
        bool first = true;
        for (auto it = table.TreeBase::begin(); it != table.TreeBase::end(); ++it) {
            if (!first) os << ",";
            os << "(" << *it << "," << it.getRef() << ")";
            first = false;
        }
        os << "]";
        return os;
    }

    friend istream& operator>>(istream& is, HashTable& table) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.setstate(ios::failbit); return is; }
        value_type key; Ref value; char comma, paren;
        while (is >> ch && ch != ']')
            if (ch == '(' && (is >> key >> comma >> value >> paren) && comma == ',' && paren == ')')
                table[key] = value;
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }
};

#endif // __HASHTABLE_H__
