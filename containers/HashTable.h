#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <functional>
#include <shared_mutex>
#include "../types.h"
#include "avl.h"
using namespace std;

//hashNode
template<typename Key, typename Value>
struct HashNode : BinaryTreeNode<Key, HashNode<Key,Value>> {
    using value_type = Key;
    Value m_value;
    HashNode() : BinaryTreeNode<Key, HashNode<Key,Value>>(Key{}, Ref{}),
                 m_value(Value{}) {}
    HashNode(Key key, Ref ref)
        : BinaryTreeNode<Key, HashNode<Key,Value>>(key, ref),
          m_value(Value{}) {}
    HashNode(Key key, Value value, Ref ref = Ref{})
        : BinaryTreeNode<Key, HashNode<Key,Value>>(key, ref),
          m_value(value) {}
    friend ostream& operator<<(ostream& os, const HashNode& n) {
        return os << n.m_data << ":" << n.m_value;
    }
};

//hashBucket
template<typename Key, typename Value>
class HashBucket : public BinaryTree<AscendingTrait<HashNode<Key,Value>>> {
public:
    using Node = HashNode<Key,Value>;

    HashBucket() : BinaryTree<AscendingTrait<HashNode<Key,Value>>>() {}

    HashBucket(const HashBucket& other)
        : BinaryTree<AscendingTrait<HashNode<Key,Value>>>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = internal_copy(static_cast<Node*>(other.m_pRoot));
    }

    HashBucket& operator=(const HashBucket& other) {
        if (this != &other) {
            this->clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = internal_copy(static_cast<Node*>(other.m_pRoot));
        }
        return *this;
    }

protected:
    Node* internal_copy(Node* pNode) override {
        if (!pNode) return nullptr;
        auto* n        = new Node(pNode->m_data, pNode->m_value, pNode->m_ref);
        n->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        n->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return n;
    }

public:
    //inserta o actualiza
    void insertKV(const Key& key, const Value& value, Ref ref = Ref{}) {
        auto* found = findNode(key);
        if (found) { found->m_value = value; return; }
        this->insert(key, ref);
        found = findNode(key);
        if (found) found->m_value = value;
    }

    //busca nodo porkey
    Node* findNode(const Key& key) const {
        Node* result = nullptr;
        const_cast<HashBucket*>(this)->inorder().forEachNode([&](Node& n) {
            if (n.m_data == key) result = &n;
        });
        return result;
    }
};

//KVPair
template<typename Key, typename Value>
struct KVPair {
    const Key& key;
    Value&     value;
};

namespace std {
    template<typename Key, typename Value>
    struct tuple_size<KVPair<Key,Value>>
        : integral_constant<size_t, 2> {};
    template<typename Key, typename Value>
    struct tuple_element<0, KVPair<Key,Value>> { using type = const Key; };
    template<typename Key, typename Value>
    struct tuple_element<1, KVPair<Key,Value>> { using type = Value; };
}
template<size_t I, typename Key, typename Value>
decltype(auto) get(KVPair<Key,Value>& p) {
    if constexpr (I == 0) return p.key; else return p.value;
}
template<size_t I, typename Key, typename Value>
decltype(auto) get(const KVPair<Key,Value>& p) {
    if constexpr (I == 0) return p.key; else return p.value;
}

//hashTable
template<typename Key, typename Value>
class HashTable {
public:
    using Node      = HashNode<Key, Value>;
    using Bucket    = HashBucket<Key, Value>;
    using MySelf    = HashTable<Key, Value>;
    using key_type  = Key;
    using mapped_type = Value;

private:
    static constexpr size_t kDefaultCapacity = 17;

    Bucket              *m_buckets;
    size_t               m_capacity;
    size_t               m_size;
    mutable shared_mutex m_mtx;

    size_t bucket_index(const Key& key) const {
        return std::hash<Key>{}(key) % m_capacity;
    }

public:
    //iterador
    struct Iterator {
        HashTable *m_table;
        size_t     m_bucket;
        size_t     m_pos;

        KVPair<Key,Value> operator*() const {
            auto view = m_table->m_buckets[m_bucket].inorder();
            auto it   = view.begin();
            for (size_t i = 0; i < m_pos; ++i) ++it;
            auto* n = it.getNode();
            return {n->m_data, n->m_value};
        }

        Iterator& operator++() {
            auto bsize = m_table->m_buckets[m_bucket].size();
            if (m_pos + 1 < bsize) { ++m_pos; return *this; }
            ++m_bucket;
            while (m_bucket < m_table->m_capacity) {
                if (m_table->m_buckets[m_bucket].size() > 0) {
                    m_pos = 0; return *this;
                }
                ++m_bucket;
            }
            m_pos = 0;
            return *this;
        }

        bool operator==(const Iterator& o) const {
            return m_bucket == o.m_bucket && m_pos == o.m_pos;
        }
        bool operator!=(const Iterator& o) const { return !(*this == o); }
    };

    Iterator begin() {
        for (size_t i = 0; i < m_capacity; ++i)
            if (m_buckets[i].size() > 0)
                return Iterator{this, i, 0};
        return end();
    }
    Iterator end() { return Iterator{this, m_capacity, 0}; }

    //constructores
    HashTable(size_t capacity = kDefaultCapacity)
        : m_buckets(new Bucket[capacity]),
          m_capacity(capacity), m_size(0) {}

    //constructor copia
    HashTable(const HashTable& other)
        : m_buckets(nullptr), m_capacity(0), m_size(0) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_capacity = other.m_capacity;
        m_size     = other.m_size;
        m_buckets  = new Bucket[m_capacity];
        for (size_t i = 0; i < m_capacity; ++i)
            m_buckets[i] = other.m_buckets[i];
    }

    //ove constructor
    HashTable(HashTable&& other)
        : m_buckets(nullptr), m_capacity(0), m_size(0) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_capacity = exchange(other.m_capacity, 0);
        m_size     = exchange(other.m_size,     0);
        m_buckets  = exchange(other.m_buckets,  nullptr);
    }

    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            shared_lock<shared_mutex> olock(other.m_mtx);
            delete[] m_buckets;
            m_capacity = other.m_capacity;
            m_size     = other.m_size;
            m_buckets  = new Bucket[m_capacity];
            for (size_t i = 0; i < m_capacity; ++i)
                m_buckets[i] = other.m_buckets[i];
        }
        return *this;
    }

    HashTable& operator=(HashTable&& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            unique_lock<shared_mutex> olock(other.m_mtx);
            delete[] m_buckets;
            m_capacity = exchange(other.m_capacity, 0);
            m_size     = exchange(other.m_size,     0);
            m_buckets  = exchange(other.m_buckets,  nullptr);
        }
        return *this;
    }

    virtual ~HashTable() { delete[] m_buckets; }

    //insert
    void insert(const Key& key, const Value& value, Ref ref = Ref{}) {
        unique_lock<shared_mutex> lock(m_mtx);
        auto idx = bucket_index(key);
        if (!m_buckets[idx].findNode(key)) ++m_size;
        m_buckets[idx].insertKV(key, value, ref);
    }

    //operator[]
    Value& operator[](const Key& key) {
        unique_lock<shared_mutex> lock(m_mtx);
        auto  idx  = bucket_index(key);
        auto* node = m_buckets[idx].findNode(key);
        if (node) return node->m_value;
        m_buckets[idx].insertKV(key, Value{});
        ++m_size;
        return m_buckets[idx].findNode(key)->m_value;
    }

    //search
    Value& search(const Key& key) {
        shared_lock<shared_mutex> lock(m_mtx);
        auto* node = m_buckets[bucket_index(key)].findNode(key);
        if (!node) throw runtime_error("key no encontrada");
        return node->m_value;
    }

    bool contains(const Key& key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_buckets[bucket_index(key)].findNode(key) != nullptr;
    }

    size_t size()    const { shared_lock<shared_mutex> lock(m_mtx); return m_size; }
    bool   isEmpty() const { shared_lock<shared_mutex> lock(m_mtx); return m_size == 0; }

    //forEach
    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        shared_lock<shared_mutex> lock(m_mtx);
        for (size_t i = 0; i < m_capacity; ++i)
            m_buckets[i].inorder().forEachNode([&](Node& n) {
                func(n.m_data, n.m_value, forward<Args>(args)...);
            });
    }

    //toString
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "{";
        bool first = true;
        for (size_t i = 0; i < m_capacity; ++i)
            m_buckets[i].inorder().forEachNode([&](Node& n) {
                if (!first) oss << ", ";
                oss << n;
                first = false;
            });
        oss << "}";
        return oss.str();
    }

    //operator<<
    friend ostream& operator<<(ostream& os, const HashTable& t) {
        return os << t.toString();
    }

    //operator>>
    friend istream& operator>>(istream& is, HashTable& t) {
        char ch;
        if (!(is >> ch) || ch != '{') { is.clear(ios_base::failbit); return is; }
        Key key; Value val; char sep;
        while (is >> ch && ch != '}') {
            if (ch != ',') is.putback(ch);
            if (is >> key >> sep >> val && sep == ':')
                t.insert(key, val);
        }
        return is;
    }
};

#endif // __HASHTABLE_H__