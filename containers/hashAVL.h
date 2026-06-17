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

template <typename PairType>
struct KeyAscendingComp {
    bool operator()(const PairType& a, const PairType& b) const {
        return a.first < b.first;
    }
};

template <typename Key, typename Value>
struct HashTrait : public BaseTrait<
    AVLNode<pair<const Key, Value>>, 
    KeyAscendingComp<pair<const Key, Value>> 
> {};

template <typename trait>
class HashTable {
public:
    using Key        = typename trait::Key;
    using Value      = typename trait::Value;
    using TreeTrait  = HashTrait<Key, Value>; 
    using value_type = typename TreeTrait::value_type; 
    
private:
    AVL<TreeTrait> m_tree; 

public:
    // Constructor
    HashTable() = default;
    // Destructor
    virtual ~HashTable() = default;

    // Copy constructor
    HashTable(const HashTable& other) : m_tree(other.m_tree) {}

    // Move constructor
    HashTable(HashTable&& other) noexcept : m_tree(std::move(other.m_tree)) {}
    
    // Copy assignment
    HashTable& operator=(const HashTable& other) {
        if (this != &other) m_tree = other.m_tree;
        return *this;
    }

    // Move assignment
    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) m_tree = std::move(other.m_tree);
        return *this;
    }

    void insert(const Key& key, const Value& value) {
        value_type new_data(key, value);
        value_type* found = m_tree.find_exact(new_data);
        
        if (found) {
            found->second = value;
        } else {
            m_tree.insert(new_data, Ref());
        }
    }
    
    Value& operator[](const Key& key) {
        value_type dummy(key, Value{});
        value_type* found = m_tree.find_exact(dummy);
        
        if (found) return found->second;
        
        m_tree.insert(dummy, Ref());
        found = m_tree.find_exact(dummy);
        return found->second;
    }

    string toString() const {
        ostringstream oss;
        oss << "{";
        bool first = true;
    
        for (const auto& [key, value] : this->inorder()) {
            if (!first) oss << ", ";
            oss << key << " : " << value;
            first = false;
        }
        
        oss << "}";
        return oss.str();
    }

    friend ostream& operator<<(ostream& os, const HashTable& table) {
        return os << table.toString();
    }

    friend istream& operator>>(istream& is, HashTable& table) {
        Char ch;
        if (!(is >> ch) || ch != '{') { 
            is.clear(ios_base::failbit); 
            return is; 
        }
        
        Key key; 
        Value val; 
        Char sep;
        
        while (is >> ch && ch != '}') {
            if (ch != ',') is.putback(ch);     
            if (is >> key >> sep >> val && sep == ':') {
                table.insert(key, val);
            }
        }
        return is;
    }

    auto inorder() const { 
        return m_tree.inorder(); 
    }
};

#endif // __HASHTABLE_H__