#ifndef __HASHAVL_H__
#define __HASHAVL_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <functional>
#include "linkedlist.h"
#include "AVL.h"
#include "traits.h"
#include "../types.h"
using namespace std;

// 1. Entrada de la cadena de colisiones
template <typename K, typename V>
struct HashAVLEntry {
    K first;
    V second;

    HashAVLEntry() : first(K()), second(V()) {}
    HashAVLEntry(const K& key)              : first(key),   second(V())    {}
    HashAVLEntry(const K& key, const V& val): first(key),   second(val)    {}

    // Comparación por clave (para el LinkedList ordenado)
    bool operator<(const HashAVLEntry& o) const { return first <  o.first; }
    bool operator>(const HashAVLEntry& o) const { return first >  o.first; }
    bool operator==(const HashAVLEntry& o) const { return first == o.first; }

    friend ostream& operator<<(ostream& os, const HashAVLEntry& e) {
        return os << "(" << e.first << "," << e.second << ")";
    }
};


// 2. Trait para el LinkedList de colisiones
template <typename K, typename V>
struct HashAVLChainTrait
    : public BaseTrait<LLNode<HashAVLEntry<K,V>>,
                       less<HashAVLEntry<K,V>>> {};

// 3. Bucket: nodo del AVL
template <typename K, typename V>
struct HashAVLBucket {
    size_t hash_val;
    LinkedList<HashAVLChainTrait<K,V>> chain;

    HashAVLBucket() : hash_val(0) {}
    HashAVLBucket(size_t h) : hash_val(h) {}

    // Solo copia/mueve hash_val; chain empieza vacía al construir por hash
    HashAVLBucket(const HashAVLBucket& o) : hash_val(o.hash_val), chain(o.chain) {}
    HashAVLBucket(HashAVLBucket&& o) noexcept
        : hash_val(o.hash_val), chain(std::move(o.chain)) {}

    // El AVL ordena los buckets por hash_val
    bool operator<(const HashAVLBucket& o) const { return hash_val <  o.hash_val; }
    bool operator>(const HashAVLBucket& o) const { return hash_val >  o.hash_val; }
    bool operator==(const HashAVLBucket& o) const { return hash_val == o.hash_val; }

    friend ostream& operator<<(ostream& os, HashAVLBucket& b) {
        for (auto it = b.chain.begin(); !(it == b.chain.end()); ++it)
            os << *it;
        return os;
    }
};

// 4. Trait del AVL: nodos son AVLNode<Bucket>
//    Ordenado por hash_val via operator
    template <typename K, typename V>
    struct HashAVLTreeTrait
        : public BaseTrait<AVLNode<HashAVLBucket<K,V>>,
                        less<HashAVLBucket<K,V>>> {};


    // 5. Búsqueda interna en el AVL por hash_val
    //    (tu AVL no expone find(), lo hacemos aquí)
template <typename K, typename V>
AVLNode<HashAVLBucket<K,V>>*
hashavl_find(AVLNode<HashAVLBucket<K,V>>* node, size_t hash_val) {
    if (!node) return nullptr;
    if (node->m_data.hash_val == hash_val) return node;
    if (hash_val < node->m_data.hash_val)
        return hashavl_find(
            static_cast<AVLNode<HashAVLBucket<K,V>>*>(node->m_pChild[0]), hash_val);
    return hashavl_find(
        static_cast<AVLNode<HashAVLBucket<K,V>>*>(node->m_pChild[1]), hash_val);
}

// 7. HashAVL
template <typename K, typename V>
class HashAVL {
public:
    using key_type    = K;
    using mapped_type = V;
    using value_type  = HashAVLEntry<K, V>;
    using Bucket      = HashAVLBucket<K, V>;
    using TreeTrait   = HashAVLTreeTrait<K, V>;
    using AVLTree     = AVL<TreeTrait>;
    using AVLNodeType = AVLNode<Bucket>;
    using ChainNode   = LLNode<value_type>;

private:
    AVLTree*             m_avl;
    size_t               m_size;
    mutable shared_mutex m_mtx;
    hash<key_type>       m_hasher;

    // ── helpers sin lock (uso interno) ──────────

    AVLNodeType* find_bucket(size_t hv) const {
        // Accedemos a m_pRoot via la vista inorder no: bajamos directo al BST
        return hashavl_find(
            static_cast<AVLNodeType*>(avl_root()), hv);
    }

    BinaryTreeNode<Bucket, AVLNode<Bucket>>* avl_root() const {
        return m_avl->root();   // ver AVLAccessor más abajo
    }

    void insert_internal(const K& key, const V& value) {
        size_t hv = m_hasher(key);

        AVLNodeType* bucket_node = find_bucket(hv);

        if (!bucket_node) {
            m_avl->insert(Bucket(hv), 0);
            bucket_node = find_bucket(hv);
        }

        auto& chain = bucket_node->m_data.chain;

        // Buscar clave exacta en la cadena de colisiones
        for (auto it = chain.begin(); !(it == chain.end()); ++it) {
            if ((*it).first == key) {
                (*it).second = value;   // actualizar valor existente
                return;
            }
        }

        // Clave nueva: agregar al final de la cadena
        chain.push_back(value_type(key, value), 0);
        ++m_size;
    }

public:
    // ── Constructores ───────────────────────────

    HashAVL() : m_avl(new AVLTree()), m_size(0) {}

    HashAVL(const HashAVL& other) : m_avl(new AVLTree()), m_size(0) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        // Recorremos inorder el AVL del otro y copiamos cada entrada
        for (auto& bucket : other.m_avl->inorder()) {
            for (auto it = const_cast<Bucket&>(bucket).chain.begin();
                 !(it == const_cast<Bucket&>(bucket).chain.end()); ++it) {
                insert_internal((*it).first, (*it).second);
            }
        }
    }

    HashAVL(HashAVL&& other) : m_avl(nullptr), m_size(0) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_avl        = exchange(other.m_avl,  new AVLTree());
        m_size       = exchange(other.m_size, 0);
    }

    HashAVL& operator=(const HashAVL& other) {
        if (this != &other) {
            delete m_avl;
            m_avl  = new AVLTree();
            m_size = 0;
            shared_lock<shared_mutex> lock(other.m_mtx);
            for (auto& bucket : other.m_avl->inorder()) {
                for (auto it = const_cast<Bucket&>(bucket).chain.begin();
                     !(it == const_cast<Bucket&>(bucket).chain.end()); ++it) {
                    insert_internal((*it).first, (*it).second);
                }
            }
        }
        return *this;
    }

    HashAVL& operator=(HashAVL&& other) {
        if (this != &other) {
            delete m_avl;
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_avl        = exchange(other.m_avl,  new AVLTree());
            m_size       = exchange(other.m_size, 0);
        }
        return *this;
    }

    virtual ~HashAVL() { delete m_avl; }

    // ── operator[] ──────────────────────────────

    mapped_type& operator[](const key_type& key) {
        unique_lock<shared_mutex> lock(m_mtx);
        size_t hv = m_hasher(key);

        AVLNodeType* bucket_node = find_bucket(hv);
        if (!bucket_node) {
            m_avl->insert(Bucket(hv), 0);
            bucket_node = find_bucket(hv);
        }

        auto& chain = bucket_node->m_data.chain;

        for (auto it = chain.begin(); !(it == chain.end()); ++it)
            if ((*it).first == key)
                return (*it).second;

        chain.push_back(value_type(key, mapped_type()), 0);
        ++m_size;

        // Segunda pasada para devolver referencia estable
        for (auto it = chain.begin(); !(it == chain.end()); ++it)
            if ((*it).first == key)
                return (*it).second;

        throw runtime_error("HashAVL::operator[]: inserción fallida");
    }

    size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    // ── ForEach ─────────────────────────────────
    // Recorre inorder el AVL y dentro de cada bucket la cadena de colisiones

    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        for (auto& bucket : m_avl->inorder()) {
            for (auto it = const_cast<Bucket&>(bucket).chain.begin();
                 !(it == const_cast<Bucket&>(bucket).chain.end()); ++it) {
                func(*it, forward<Args>(args)...);
            }
        }
    }

    // ── Streams ─────────────────────────────────

    friend ostream& operator<<(ostream& os, HashAVL& h) {
        shared_lock<shared_mutex> lock(h.m_mtx);
        os << "[";
        bool first = true;
        for (auto& bucket : h.m_avl->inorder()) {
            for (auto it = bucket.chain.begin();
                 !(it == bucket.chain.end()); ++it) {
                if (!first) os << ",";
                os << *it;                                          
                first = false;
            }
        }
        return os << "]";
    }

    friend istream& operator>>(istream& is, HashAVL& h)
    {
        Char ch;
        if (!(is >> ch) || ch != '[')
        {
            is.setstate(ios::failbit);
            return is;
        }
        while (is >> ch)
        {
            if (ch == ']')  break;
            if (ch != '(')    continue;
            K key;
            Char comma;

            if (!(is >> key >> comma) || comma != ',')
            {
                is.setstate(ios::failbit);
                return is;
            }

            Cadena value;
            getline(is, value, ')');

            h[key] = value;

            if (is.peek() == ',')   is.get();
        }
        return is;
    }
};

#endif // __HASHAVL_H__