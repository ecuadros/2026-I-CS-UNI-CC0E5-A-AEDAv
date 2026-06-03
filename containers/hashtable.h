#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <iostream>
#include <string>
#include <sstream>
#include <shared_mutex>
#include <mutex>
#include <tuple>
#include <utility>
#include "../types.h"
#include "avl.h"
using namespace std;

// Iterador key-value: envuelve el forward inorder del arbol y expone (key, value&)
// para structured bindings. No toca general_iterator (afectaria a todos).
template<typename Node>
class HashIterator {
    TreeForwardIterator<Node> m_it;
public:
    HashIterator(TreeForwardIterator<Node> it) : m_it(move(it)) {}
    tuple<typename Node::value_type, Ref&> operator*() {
        Node* n = m_it.node();
        return { n->m_data, n->m_ref };
    }
    HashIterator& operator++() { ++m_it; return *this; }
    bool operator!=(const HashIterator& o) const { return m_it != o.m_it; }
    bool operator==(const HashIterator& o) const { return m_it == o.m_it; }
};

template<typename Trait>
class HashTable : public AVL<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using AVL<Trait>::AVL;

    HashTable() = default;
    HashTable(const HashTable& other) : AVL<Trait>(other) {}
    HashTable(HashTable&& other) noexcept : AVL<Trait>(move(other)) {}

    // Semantica de mapa: si la clave existe devuelve su ref; si no, la inserta.
    Ref& operator[](const value_type& key) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = this->find_node(key);
        if(!n) {
            this->internal_insert(this->m_pRoot, key, Ref());
            n = this->find_node(key);
        }
        return n->m_ref;
    }

    HashIterator<Node> begin() { return HashIterator<Node>(this->inorder().m_begin); }
    HashIterator<Node> end()   { return HashIterator<Node>(this->inorder().m_end);   }

    string toString() {
        ostringstream oss;
        oss << "{";
        bool first = true;
        for(auto it = begin(); it != end(); ++it) {
            auto [k, v] = *it;
            if(!first) oss << ", ";
            oss << k << ": " << v;
            first = false;
        }
        oss << "}";
        return oss.str();
    }

    friend ostream& operator<<(ostream& os, HashTable& m) {
        return os << m.toString();
    }

    friend istream& operator>>(istream& is, HashTable& m) {
        char ch;
        if(!(is >> ch) || ch != '{') { is.clear(ios_base::failbit); return is; }
        value_type key; Ref val; char colon;
        while(is >> ch) {
            if(ch == '}') break;
            if(ch == ',') continue;
            is.putback(ch);
            if(is >> key >> colon >> val && colon == ':')
                m[key] = val;
            else break;
        }
        return is;
    }
};

#endif // __HASHTABLE_H__
