#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <iostream>
#include <vector>
#include <utility>
#include <tuple>
#include <functional>
#include <mutex>
#include <shared_mutex>

#include "AVL.h"
#include "general_iterator.h"
#include "../types.h"
using namespace std;

template <typename K>
struct DefaultHash {
    size_t operator()(const K& key) const { return std::hash<K>{}(key); }
};

template <typename T, typename _Comp, typename NodeType, typename _Hasher, template<typename> class _Bucket>
struct BaseHashTrait : BaseTrait<T, _Comp, NodeType> {
    using Hasher = _Hasher;
    template<typename Tr> using Bucket = _Bucket<Tr>;
};

template <typename T>
struct AscendingHashTrait  : BaseHashTrait<T, less<T>,    AVLNode<T>, DefaultHash<T>, AVL> {};
template <typename T>
struct DescendingHashTrait : BaseHashTrait<T, greater<T>, AVLNode<T>, DefaultHash<T>, AVL> {};

template <typename Trait> class HashTable;

// ToDo for (const auto& [key, value] : m)
template <typename Container>
class hash_forward_iterator
    : public general_iterator<Container, hash_forward_iterator<Container>> {
    using Node       = typename Container::Node;
    using value_type = typename Container::value_type;
    vector<Node*> m_nodes;
    size_t        m_idx;
public:
    using MySelf = hash_forward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    hash_forward_iterator(Container* c, vector<Node*> nodes, size_t idx)
        : Parent(c, nullptr), m_nodes(move(nodes)), m_idx(idx) {
        if (m_idx < m_nodes.size()) this->m_pNode = m_nodes[m_idx];
    }
    MySelf operator++() {
        ++m_idx;
        this->m_pNode = (m_idx < m_nodes.size()) ? m_nodes[m_idx] : nullptr;
        return *this;
    }
    tuple<value_type, Ref&> operator*() {
        return tuple<value_type, Ref&>(this->m_pNode->m_data, this->m_pNode->m_ref);
    }
};

template <typename Trait>
class HashTable {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Hasher     = typename Trait::Hasher;
    using Bucket     = typename Trait::template Bucket<Trait>;
    using MySelf     = HashTable<Trait>;
    using iterator   = hash_forward_iterator<MySelf>;

    friend iterator;

private:
    Bucket*              m_buckets;
    size_t               m_numBuckets;
    Hasher               m_hash;
    mutable shared_mutex m_mtx;

    size_t indexOf(const value_type& key) const {
        return m_hash(key) % m_numBuckets;
    }

    void allocate(size_t n) {
        m_numBuckets = (n == 0) ? 1 : n;
        m_buckets    = new Bucket[m_numBuckets];
    }

    vector<Node*> collect() const {
        vector<Node*> nodes;
        for (size_t i = 0; i < m_numBuckets; ++i)
            for (auto it = m_buckets[i].begin(); it != m_buckets[i].end(); ++it)
                nodes.push_back(it.getNode());
        return nodes;
    }

public:
    explicit HashTable(size_t numBuckets = 8) { allocate(numBuckets); }

    ~HashTable() {
        unique_lock<shared_mutex> lock(m_mtx);
        delete[] m_buckets;
        m_buckets = nullptr;
    }

    // ToDo Constructor copia
    HashTable(const HashTable& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_hash = other.m_hash;
        allocate(other.m_numBuckets);
        for (size_t i = 0; i < m_numBuckets; ++i)
            m_buckets[i] = other.m_buckets[i];
    }

    // ToDo Move constructor
    HashTable(HashTable&& other) noexcept {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_buckets    = exchange(other.m_buckets, nullptr);
        m_numBuckets = exchange(other.m_numBuckets, 0);
        m_hash       = other.m_hash;
    }

    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lockSelf(m_mtx);
            shared_lock<shared_mutex> lockOther(other.m_mtx);
            delete[] m_buckets;
            m_hash = other.m_hash;
            allocate(other.m_numBuckets);
            for (size_t i = 0; i < m_numBuckets; ++i)
                m_buckets[i] = other.m_buckets[i];
        }
        return *this;
    }

    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            unique_lock<shared_mutex> lockSelf(m_mtx);
            unique_lock<shared_mutex> lockOther(other.m_mtx);
            delete[] m_buckets;
            m_buckets    = exchange(other.m_buckets, nullptr);
            m_numBuckets = exchange(other.m_numBuckets, 0);
            m_hash       = other.m_hash;
        }
        return *this;
    }

    // ToDo operador[];
    Ref& operator[](const value_type& key) {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_buckets[indexOf(key)][key];
    }

    void insert(const value_type& key, Ref ref) {
        shared_lock<shared_mutex> lock(m_mtx);
        m_buckets[indexOf(key)][key] = ref;
    }

    bool contains(const value_type& key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_buckets[indexOf(key)].contains(key);
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        size_t total = 0;
        for (size_t i = 0; i < m_numBuckets; ++i)
            total += m_buckets[i].size();
        return total;
    }

    size_t bucketCount() const { shared_lock<shared_mutex> lock(m_mtx); return m_numBuckets; }
    size_t indexFor(const value_type& key) const { shared_lock<shared_mutex> lock(m_mtx); return indexOf(key); }
    string bucketToString(size_t i) const { shared_lock<shared_mutex> lock(m_mtx); return m_buckets[i].toString(); }

    double loadFactor() const {
        shared_lock<shared_mutex> lock(m_mtx);
        size_t total = 0;
        for (size_t i = 0; i < m_numBuckets; ++i) total += m_buckets[i].size();
        return static_cast<double>(total) / static_cast<double>(m_numBuckets);
    }

    iterator begin() { return iterator(this, collect(), 0); }
    iterator end()   { return iterator(this, {}, 0); }

    // ToDo operator<<
    friend ostream& operator<<(ostream& os, HashTable& table) {
        shared_lock<shared_mutex> lock(table.m_mtx);
        os << "[";
        bool first = true;
        for (size_t i = 0; i < table.m_numBuckets; ++i)
            for (auto it = table.m_buckets[i].begin(); it != table.m_buckets[i].end(); ++it) {
                if (!first) os << ",";
                os << "(" << *it << "," << it.getRef() << ")";
                first = false;
            }
        os << "]";
        return os;
    }

    // ToDo operator>>
    friend istream& operator>>(istream& is, HashTable& table) {
        Bucket tmp;
        is >> tmp;
        for (auto it = tmp.begin(); it != tmp.end(); ++it)
            table.insert(*it, it.getRef());
        return is;
    }
};

#endif // __HASHTABLE_H__
