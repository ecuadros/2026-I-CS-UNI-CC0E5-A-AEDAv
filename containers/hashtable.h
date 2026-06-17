#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <iostream>
#include <string>
#include <sstream>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include "../types.h"
#include "avl.h"
#include "linkedlist.h"
using namespace std;

// HashEntry<Key, Value>
// entrada (key, value) de la cadena de colisiones. "key: value"
template<typename Key, typename Value>
struct HashEntry {
    Key   m_key;
    Value m_value;

    HashEntry() : m_key(), m_value() {}
    HashEntry(const Key& k, const Value& v = Value{}) : m_key(k), m_value(v) {}

    bool operator<(const HashEntry& o)  const { return m_key <  o.m_key; }
    bool operator>(const HashEntry& o)  const { return m_key >  o.m_key; }
    bool operator==(const HashEntry& o) const { return m_key == o.m_key; }

    friend ostream& operator<<(ostream& os, const HashEntry& e) { return os << e.m_key << ": " << e.m_value; }
};

// trait de la cadena de colisiones (LinkedList por bucket)
template<typename Key, typename Value>
struct HashChainTrait : BaseTrait<LLNode<HashEntry<Key, Value>>, less<HashEntry<Key, Value>>> {};

// HashBucket<Key, Value>
// dato de cada nodo del AVL: hash + cadena de colisiones
template<typename Key, typename Value>
struct HashBucket {
    size_t m_hash;
    LinkedList<HashChainTrait<Key, Value>> m_chain;

    HashBucket() : m_hash(0) {}
    HashBucket(size_t h) : m_hash(h) {}

    bool operator<(const HashBucket& o)  const { return m_hash <  o.m_hash; }
    bool operator>(const HashBucket& o)  const { return m_hash >  o.m_hash; }
    bool operator==(const HashBucket& o) const { return m_hash == o.m_hash; }

    // el bucket imprime su cadena (reusa operator<< de LinkedList)
    friend ostream& operator<<(ostream& os, const HashBucket& b) { return os << b.m_chain; }
};

// HashTrait
// reusa AscendingAVLTrait + agrega los tipos del hash. H = funcion hash
template<typename Key, typename Value, typename H = hash<Key>>
struct HashTrait : AscendingAVLTrait<HashBucket<Key, Value>> {
    using key_type    = Key;
    using mapped_type = Value;
    using Entry       = HashEntry<Key, Value>;
    using Hasher      = H;
};

// HashIterator
// inorder del AVL (buckets) + el del LinkedList (cadena del bucket)
template<typename Trait>
class HashIterator {
public:
    using Bucket    = typename Trait::value_type;
    using Entry     = typename Trait::Entry;
    using BucketIt  = typename BinaryTree<Trait>::inorder_fwd;
    using Chain     = LinkedList<HashChainTrait<typename Trait::key_type, typename Trait::mapped_type>>;
    using ChainNode = typename Chain::Node;     // LLNode<HashEntry>

private:
    BucketIt   m_bIt, m_bEnd;
    ChainNode* m_node;     // nodo actual de la cadena (nullptr = cadena agotada)

    // si la cadena actual se agoto, salta al siguiente bucket no vacio
    void skipEmpty() {
        while(m_bIt != m_bEnd && m_node == nullptr) {
            ++m_bIt;
            if(m_bIt != m_bEnd)
                m_node = (*m_bIt).m_chain.begin().getNode();   // primer nodo del bucket
        }
    }

public:
    HashIterator(BucketIt b, BucketIt e) : m_bIt(b), m_bEnd(e), m_node(nullptr) {
        if(m_bIt != m_bEnd) {
            m_node = (*m_bIt).m_chain.begin().getNode();
            skipEmpty();
        }
    }

    Entry& operator*()         { return m_node->getDataRef(); }
    HashIterator& operator++() { m_node = m_node->getNext(); skipEmpty(); return *this; }
    bool operator==(const HashIterator& o) const {
        if(m_bIt != o.m_bIt) return false;
        if(m_bIt == m_bEnd)  return true;
        return m_node == o.m_node;
    }
    bool operator!=(const HashIterator& o) const { return !(*this == o); }
};

// HashTable<Trait>
// hash en un AVL: hash(key) -> bucket -> cadena de colisiones
template<typename Trait>
class HashTable : public AVL<Trait> {
public:
    using Base        = AVL<Trait>;
    using Node        = typename Base::Node;          // AVLNode<HashBucket>
    using Bucket      = typename Trait::value_type;   // HashBucket
    using key_type    = typename Trait::key_type;
    using mapped_type = typename Trait::mapped_type;
    using Entry       = typename Trait::Entry;        // HashEntry
    using Hasher      = typename Trait::Hasher;

private:
    Hasher m_hash;

    // ubica el bucket por su hash
    Node* find_bucket(size_t h) const {
        Bucket probe(h);
        Node* n = this->m_pRoot;
        while(n) {
            if(!this->m_comp(n->m_data, probe) && !this->m_comp(probe, n->m_data))
                return n;
            n = n->m_pChild[!this->m_comp(n->m_data, probe)];
        }
        return nullptr;
    }

public:
    HashTable() {}
    HashTable(const HashTable& other) : Base(other) {}              // copia 
    HashTable(HashTable&& other) noexcept : Base(move(other)) {}    // move

    // m = {{1,"a"}, {2,"b"}}
    HashTable(initializer_list<Entry> init) {
        for(const auto& e : init)
            (*this)[e.m_key] = e.m_value;
    }

    // m[key]: ubica/crea el bucket por hash y busca la key en la cadena
    mapped_type& operator[](const key_type& key) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        size_t h = m_hash(key);
        Node* bnode = find_bucket(h);
        if(!bnode) {
            this->internal_insert(this->m_pRoot, Bucket(h), Ref{}, nullptr);
            bnode = find_bucket(h);                       // re-localiza
        }
        auto& chain = bnode->m_data.m_chain;
        for(auto& e : chain)
            if(e.m_key == key) return e.m_value;          // ya existe
        chain.push_back(Entry(key), (Ref)h);              // colision/nueva
        for(auto& e : chain)
            if(e.m_key == key) return e.m_value;
        throw runtime_error("HashTable::operator[]: insert fallo");
    }

    // at 
    mapped_type& at(const key_type& key) {
        shared_lock<shared_mutex> lock(this->m_mtx);
        Node* bnode = find_bucket(m_hash(key));
        if(bnode)
            for(auto& e : bnode->m_data.m_chain)
                if(e.m_key == key) return e.m_value;
        throw out_of_range("HashTable::at: key no existe");
    }
    const mapped_type& at(const key_type& key) const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        Node* bnode = find_bucket(m_hash(key));
        if(bnode)
            for(auto& e : bnode->m_data.m_chain)
                if(e.m_key == key) return e.m_value;
        throw out_of_range("HashTable::at: key no existe");
    }

    bool contains(const key_type& key) const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        Node* bnode = find_bucket(m_hash(key));
        if(!bnode) return false;
        for(auto& e : bnode->m_data.m_chain)
            if(e.m_key == key) return true;
        return false;
    }

    // recorre las entradas por el iterador del AVL + la cadena
    template<typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        this->inorder().forEach([&](Bucket& bucket) {
            for(auto& e : bucket.m_chain)
                func(e, forward<Args>(args)...);
        });
    }

    // begin/end con el iterador aplanador -> habilita for (const auto& [k,v] : m)
    HashIterator<Trait> begin() { return HashIterator<Trait>(Base::begin(), Base::end()); }
    HashIterator<Trait> end()   { return HashIterator<Trait>(Base::end(),   Base::end()); }

    // {k: v, ...}: reusa el recorrido del AVL + la cadena
    friend ostream& operator<<(ostream& os, HashTable& h) {
        os << "{";
        bool first = true;
        h.ForEach([&](const Entry& e) {
            if(!first) os << ", ";
            os << e;                  // "key: value"
            first = false;
        });
        return os << "}";
    }

    // lee {k: v, ...} y arma la tabla con operator[]
    friend istream& operator>>(istream& is, HashTable& h) {
        Token ch;
        if(!(is >> ch) || ch != '{') { is.clear(ios_base::failbit); return is; }
        while((is >> ws).peek() != '}') {
            key_type key; Token colon;
            if(!(is >> key >> colon) || colon != ':') break;
            string raw; Token c = '\0';
            while(is.get(c) && c != ',' && c != '}') raw += c;   // valor hasta , o }
            size_t a = raw.find_first_not_of(" \t");
            size_t b = raw.find_last_not_of(" \t");
            raw = (a == string::npos) ? string() : raw.substr(a, b - a + 1);
            mapped_type val{};
            istringstream iss(raw); iss >> val;
            h[key] = val;
            if(c == '}') return is;
        }
        is >> ch;
        return is;
    }
};

#endif // __HASHTABLE_H__
