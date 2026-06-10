#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include "avl.h"
#include "traits.h"
#include "vector.h"
using namespace std;

template<typename Key, typename Value>
using HashEntry = pair<Key, Value>;

template<typename Key, typename Value>
struct HashNode : public AVLNode<Key, HashNode<Key, Value>> {
    using Base = AVLNode<Key, HashNode<Key, Value>>;
    using value_type = Key;
    using Entry = HashEntry<Key, Value>;

    Entry entry;

    HashNode(Key key, Ref ref) : Base(key, ref), entry(key, Value()) {}
    HashNode(Key key, Value value, Ref ref = Ref{}) : Base(key, ref), entry(key, value) {}

    friend ostream& operator<<(ostream &os, const HashNode &node) {
        return os << "(" << node.entry.first << "," << node.entry.second << ")";
    }
};

template<typename Key, typename Value>
class HashBucket : public AVL<AscendingTrait<HashNode<Key, Value>>> {
public:
    using Base = AVL<AscendingTrait<HashNode<Key, Value>>>;
    using Node = HashNode<Key, Value>;
    using Entry = typename Node::Entry;

    HashBucket() : Base() {}

    HashBucket(const HashBucket &other) : Base() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_comp = other.m_comp;
        this->m_pRoot = internal_copy(other.m_pRoot);
    }

    HashBucket(HashBucket &&other) : Base(std::move(other)) {}

    HashBucket& operator=(const HashBucket &other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(this->m_mtx);
            shared_lock<shared_mutex> otherLock(other.m_mtx);
            this->internal_clear(this->m_pRoot);
            this->m_comp = other.m_comp;
            this->m_pRoot = internal_copy(other.m_pRoot);
        }
        return *this;
    }

    HashBucket& operator=(HashBucket &&other) {
        Base::operator=(std::move(other));
        return *this;
    }

protected:
    Node* internal_copy(Node *pNode) const override {
        if (!pNode) {
            return nullptr;
        }

        Node *newNode = new Node(pNode->entry.first, pNode->entry.second, pNode->m_ref);
        newNode->m_height = pNode->m_height;
        newNode->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        newNode->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return newNode;
    }

private:
    Node* findNodeUnlocked(const Key &key) const {
        return this->internal_search(this->m_pRoot, key);
    }

    void collect(Node *node, RefVector<const Entry*> &items) const {
        if (!node) {
            return;
        }

        collect(node->m_pChild[0], items);
        items.push_back(&node->entry, 0);
        collect(node->m_pChild[1], items);
    }

    void append(Node *node, ostringstream &oss, bool &first) const {
        if (!node) {
            return;
        }

        append(node->m_pChild[0], oss, first);
        if (!first) {
            oss << ",";
        }
        oss << "(" << node->entry.first << "," << node->entry.second << ")";
        first = false;
        append(node->m_pChild[1], oss, first);
    }

public:
    bool insertKV(const Key &key, const Value &value, Ref ref = Ref{}) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *found = findNodeUnlocked(key);
        if (found) {
            found->entry.second = value;
            return false;
        }

        this->internal_insert(this->m_pRoot, key, ref);
        found = findNodeUnlocked(key);
        if (found) {
            found->entry.second = value;
        }
        return true;
    }

    Value& getOrInsert(const Key &key, bool &inserted) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *found = findNodeUnlocked(key);
        if (found) {
            inserted = false;
            return found->entry.second;
        }

        this->internal_insert(this->m_pRoot, key, Ref{});
        inserted = true;
        return findNodeUnlocked(key)->entry.second;
    }

    Value at(const Key &key) const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        Node *found = findNodeUnlocked(key);
        if (!found) {
            throw out_of_range("Clave no encontrada");
        }
        return found->entry.second;
    }

    bool containsKey(const Key &key) const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return findNodeUnlocked(key) != nullptr;
    }

    void collect(RefVector<const Entry*> &items) const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        collect(this->m_pRoot, items);
    }

    string toString() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        ostringstream oss;
        bool first = true;
        oss << "[";
        append(this->m_pRoot, oss, first);
        oss << "]";
        return oss.str();
    }
};

template <
    typename Key,
    typename Value,
    typename Hash = hash<Key>
>
class HashTable {
public:
    using key_type = Key;
    using mapped_type = Value;
    using Bucket = HashBucket<Key, Value>;
    using Entry = typename Bucket::Entry;
    using MySelf = HashTable<Key, Value, Hash>;

    class const_iterator {
        RefVector<const Entry*> m_items;
        size_t m_index = 0;

    public:
        const_iterator(const RefVector<const Entry*> &items, size_t index)
            : m_items(items), m_index(index) {}

        const Entry& operator*() const {
            return *m_items[m_index];
        }

        const Entry* operator->() const {
            return m_items[m_index];
        }

        const_iterator& operator++() {
            ++m_index;
            return *this;
        }

        bool operator==(const const_iterator &other) const {
            return m_index == other.m_index && m_items.size() == other.m_items.size();
        }

        bool operator!=(const const_iterator &other) const {
            return !(*this == other);
        }
    };

private:
    Bucket              *m_buckets;
    size_t               m_capacity;
    size_t               m_size;
    Hash                 m_hash;
    mutable shared_mutex m_mtx;

    size_t bucketIndex(const Key &key) const {
        return m_hash(key) % m_capacity;
    }

    void collectEntries(RefVector<const Entry*> &items) const {
        for (size_t i = 0; i < m_capacity; ++i) {
            m_buckets[i].collect(items);
        }
    }

public:
    HashTable(size_t capacity = 16)
        : m_buckets(new Bucket[capacity == 0 ? 1 : capacity]),
          m_capacity(capacity == 0 ? 1 : capacity),
          m_size(0),
          m_hash() {}

    HashTable(const MySelf &other)
        : m_buckets(nullptr), m_capacity(other.m_capacity), m_size(other.m_size), m_hash(other.m_hash) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_buckets = new Bucket[m_capacity];
        for (size_t i = 0; i < m_capacity; ++i) {
            m_buckets[i] = other.m_buckets[i];
        }
    }

    HashTable(MySelf &&other)
        : m_buckets(nullptr), m_capacity(0), m_size(0), m_hash(std::move(other.m_hash)) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_buckets = std::exchange(other.m_buckets, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_size = std::exchange(other.m_size, 0);
    }

    MySelf& operator=(const MySelf &other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            shared_lock<shared_mutex> otherLock(other.m_mtx);
            Bucket *newBuckets = new Bucket[other.m_capacity];
            for (size_t i = 0; i < other.m_capacity; ++i) {
                newBuckets[i] = other.m_buckets[i];
            }
            delete [] m_buckets;
            m_buckets = newBuckets;
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            m_hash = other.m_hash;
        }
        return *this;
    }

    MySelf& operator=(MySelf &&other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            unique_lock<shared_mutex> otherLock(other.m_mtx);
            delete [] m_buckets;
            m_buckets = std::exchange(other.m_buckets, nullptr);
            m_capacity = std::exchange(other.m_capacity, 0);
            m_size = std::exchange(other.m_size, 0);
            m_hash = std::move(other.m_hash);
        }
        return *this;
    }

    ~HashTable() {
        delete [] m_buckets;
    }

    bool insert(const Key &key, const Value &value) {
        unique_lock<shared_mutex> lock(m_mtx);
        bool inserted = m_buckets[bucketIndex(key)].insertKV(key, value);
        if (inserted) {
            ++m_size;
        }
        return inserted;
    }

    Value& operator[](const Key &key) {
        unique_lock<shared_mutex> lock(m_mtx);
        bool inserted = false;
        Value &value = m_buckets[bucketIndex(key)].getOrInsert(key, inserted);
        if (inserted) {
            ++m_size;
        }
        return value;
    }

    Value at(const Key &key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_buckets[bucketIndex(key)].at(key);
    }

    bool contains(const Key &key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_buckets[bucketIndex(key)].containsKey(key);
    }

    bool isEmpty() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size == 0;
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    size_t capacity() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_capacity;
    }

    const_iterator begin() const {
        shared_lock<shared_mutex> lock(m_mtx);
        RefVector<const Entry*> items(m_size + 1);
        collectEntries(items);
        return const_iterator(items, 0);
    }

    const_iterator end() const {
        shared_lock<shared_mutex> lock(m_mtx);
        RefVector<const Entry*> items(m_size + 1);
        collectEntries(items);
        return const_iterator(items, items.size());
    }

    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "{";
        bool firstBucket = true;
        for (size_t i = 0; i < m_capacity; ++i) {
            if (m_buckets[i].size() == 0) {
                continue;
            }
            if (!firstBucket) {
                oss << ",";
            }
            oss << i << ":" << m_buckets[i].toString();
            firstBucket = false;
        }
        oss << "}";
        return oss.str();
    }

    bool operator<(const MySelf &other) const {
        return size() < other.size();
    }

    bool operator>(const MySelf &other) const {
        return size() > other.size();
    }

    friend ostream& operator<<(ostream &os, const MySelf &table) {
        return os << table.toString();
    }

    friend istream& operator>>(istream &is, MySelf &table) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return is;
        }

        Key key;
        Value value;
        char comma;
        char closeParen;

        while (is >> ch && ch != ']') {
            if (ch == '(' && is >> key >> comma >> value >> closeParen) {
                if (comma == ',' && closeParen == ')') {
                    table.insert(key, value);
                }
            }
        }
        return is;
    }
};

void HashTableDemo();

#endif // __HASHTABLE_H__
