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
    using key_type = Key;
    using mapped_type = Value;
    using value_type = key_type;
    using Entry = HashEntry<key_type, mapped_type>;

    Entry entry;

    HashNode(key_type key, Ref ref) : Base(key, ref), entry(key, mapped_type{}) {}
    HashNode(key_type key, mapped_type value, Ref ref = Ref{}) : Base(key, ref), entry(key, value) {}

    friend ostream& operator<<(ostream &os, const HashNode &node) {
        return os << "(" << node.entry.first << "," << node.entry.second << ")";
    }
};

template<typename Node>
struct HashTableTrait : public BaseTrait<Node, less<typename Node::key_type>> {
    using key_type = typename Node::key_type;
    using mapped_type = typename Node::mapped_type;
};

template<typename Trait>
class HashBucket : public AVL<Trait> {
public:
    using Base = AVL<Trait>;
    using key_type = typename Trait::key_type;
    using mapped_type = typename Trait::mapped_type;
    using Node = typename Trait::Node;
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
            Node *newRoot = nullptr;
            typename Base::Comp newComp;
            {
                shared_lock<shared_mutex> otherLock(other.m_mtx);
                newComp = other.m_comp;
                newRoot = internal_copy(other.m_pRoot);
            }

            unique_lock<shared_mutex> lock(this->m_mtx);
            this->internal_clear(this->m_pRoot);
            this->m_comp = newComp;
            this->m_pRoot = newRoot;
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
    Node* findNodeUnlocked(const key_type &key) const {
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
    bool insertKV(const key_type &key, const mapped_type &value, Ref ref = Ref{}) {
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

    mapped_type& getOrInsert(const key_type &key, bool &inserted) {
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

    mapped_type at(const key_type &key) const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        Node *found = findNodeUnlocked(key);
        if (!found) {
            throw out_of_range("Clave no encontrada");
        }
        return found->entry.second;
    }

    bool containsKey(const key_type &key) const {
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

template <typename Trait>
class HashTable {
public:
    using key_type = typename Trait::key_type;
    using mapped_type = typename Trait::mapped_type;
    using Hash = hash<key_type>;
    using Bucket = HashBucket<Trait>;
    using Entry = typename Bucket::Entry;
    using MySelf = HashTable<Trait>;

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
    Bucket              *m_buckets = nullptr;
    size_t               m_capacity = 0;
    size_t               m_size = 0;
    Hash                 m_hash{};
    mutable shared_mutex m_mtx;

    size_t bucketIndex(const key_type &key) const {
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
          m_capacity(capacity == 0 ? 1 : capacity) {}

    HashTable(const MySelf &other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_capacity = other.m_capacity;
        m_size = other.m_size;
        m_hash = other.m_hash;
        m_buckets = new Bucket[m_capacity];
        for (size_t i = 0; i < m_capacity; ++i) {
            m_buckets[i] = other.m_buckets[i];
        }
    }

    HashTable(MySelf &&other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_buckets = std::exchange(other.m_buckets, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_size = std::exchange(other.m_size, 0);
        m_hash = std::move(other.m_hash);
    }

    MySelf& operator=(const MySelf &other) {
        if (this == &other) {
            return *this;
        }

        Bucket *newBuckets = nullptr;
        size_t newCapacity;
        size_t newSize;
        Hash newHash;
        {
            shared_lock<shared_mutex> otherLock(other.m_mtx);
            newCapacity = other.m_capacity;
            newSize = other.m_size;
            newHash = other.m_hash;
            newBuckets = new Bucket[newCapacity];
            for (size_t i = 0; i < newCapacity; ++i) {
                newBuckets[i] = other.m_buckets[i];
            }
        }

        unique_lock<shared_mutex> lock(m_mtx);
        delete [] m_buckets;
        m_buckets = newBuckets;
        m_capacity = newCapacity;
        m_size = newSize;
        m_hash = newHash;
        return *this;
    }

    MySelf& operator=(MySelf &&other) {
        if (this == &other) {
            return *this;
        }

        unique_lock<shared_mutex> lock(m_mtx, defer_lock);
        unique_lock<shared_mutex> otherLock(other.m_mtx, defer_lock);
        std::lock(lock, otherLock);
        delete [] m_buckets;
        m_buckets = std::exchange(other.m_buckets, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_size = std::exchange(other.m_size, 0);
        m_hash = std::move(other.m_hash);
        return *this;
    }

    ~HashTable() {
        delete [] m_buckets;
    }

    bool insert(const key_type &key, const mapped_type &value) {
        unique_lock<shared_mutex> lock(m_mtx);
        bool inserted = m_buckets[bucketIndex(key)].insertKV(key, value);
        if (inserted) {
            ++m_size;
        }
        return inserted;
    }

    mapped_type& operator[](const key_type &key) {
        unique_lock<shared_mutex> lock(m_mtx);
        bool inserted = false;
        mapped_type &value = m_buckets[bucketIndex(key)].getOrInsert(key, inserted);
        if (inserted) {
            ++m_size;
        }
        return value;
    }

    mapped_type at(const key_type &key) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_buckets[bucketIndex(key)].at(key);
    }

    bool contains(const key_type &key) const {
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

        key_type key;
        mapped_type value;
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
