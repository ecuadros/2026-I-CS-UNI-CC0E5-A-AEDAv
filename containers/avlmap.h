#ifndef __AVLMAP_H__
#define __AVLMAP_H__

#include <iostream>
#include <string>
#include <sstream>
#include <tuple>
#include <utility>
#include <initializer_list>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>
#include "../types.h"
#include "avl.h"
using namespace std;

// ─── KVPair<Key, Value> ──────────────────────────────────────────────────────
// par (key, value). el arbol se ordena por la key
template<typename Key, typename Value>
struct KVPair {
    Key   m_key;
    Value m_value;

    KVPair() : m_key(), m_value() {}
    KVPair(const Key& k, const Value& v = Value{}) : m_key(k), m_value(v) {}

    bool operator<(const KVPair& o)  const { return m_key <  o.m_key; }
    bool operator>(const KVPair& o)  const { return m_key >  o.m_key; }
    bool operator==(const KVPair& o) const { return m_key == o.m_key; }

    friend ostream& operator<<(ostream& os, const KVPair& p) { return os << p.m_key << ": " << p.m_value; }
    friend istream& operator>>(istream& is, KVPair& p) { Token c; return is >> p.m_key >> c >> p.m_value; }
};

// ─── tuple para [key, value] ─────────────────────────────────────────────────
// para poder hacer: for (const auto& [k, v] : m)
namespace std {
    template<typename K, typename V>
    struct tuple_size<::KVPair<K, V>> : integral_constant<size_t, 2> {};
    template<typename K, typename V>
    struct tuple_element<0, ::KVPair<K, V>> { using type = const K; };
    template<typename K, typename V>
    struct tuple_element<1, ::KVPair<K, V>> { using type = V; };
}
template<size_t I, typename K, typename V>
decltype(auto) get(KVPair<K, V>& p) {
    if constexpr(I == 0) return (const K&)p.m_key;
    else                 return (V&)p.m_value;
}
template<size_t I, typename K, typename V>
decltype(auto) get(const KVPair<K, V>& p) {
    if constexpr(I == 0) return (const K&)p.m_key;
    else                 return (const V&)p.m_value;
}

// ─── AVLMap<Key, Value> ──────────────────────────────────────────────────────
// mapa ordenado sobre AVL: reusa el balanceo y la concurrencia del arbol
template<typename Key, typename Value>
class AVLMap : public AVL<AscendingAVLTrait<KVPair<Key, Value>>> {
public:
    using Pair       = KVPair<Key, Value>;
    using Base       = AVL<AscendingAVLTrait<Pair>>;
    using Node       = typename Base::Node;
    using value_type = Pair;

private:
    // corre bajo el lock del método público (operator[]/at)
    Node* find_node(const Key& key) const {
        Pair probe(key);
        Node* n = this->m_pRoot;
        while(n) {
            if(!this->m_comp(n->m_data, probe) && !this->m_comp(probe, n->m_data))
                return n;
            n = n->m_pChild[!this->m_comp(n->m_data, probe)];
        }
        return nullptr;
    }

public:
    AVLMap() {}
    AVLMap(const AVLMap& other) : Base(other) {}              // copia
    AVLMap(AVLMap&& other) noexcept : Base(move(other)) {}    // move

    // permite: m = {{1,"a"}, {2,"b"}}
    AVLMap(initializer_list<Pair> init) {
        for(const auto& p : init)
            this->internal_insert(this->m_pRoot, p, Ref{}, nullptr);
    }

    // si la key existe devuelve su valor. si no, la crea
    Value& operator[](const Key& key) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = find_node(key);
        if(!n) {
            this->internal_insert(this->m_pRoot, Pair(key), Ref{}, nullptr);
            n = find_node(key);
        }
        return n->m_data.m_value;
    }

    // como [] pero lanza si la key no existe
    Value& at(const Key& key) {
        shared_lock<shared_mutex> lock(this->m_mtx);
        Node* n = find_node(key);
        if(!n) throw out_of_range("AVLMap::at: key no existe");
        return n->m_data.m_value;
    }
    const Value& at(const Key& key) const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        const Node* n = find_node(key);
        if(!n) throw out_of_range("AVLMap::at: key no existe");
        return n->m_data.m_value;
    }

    bool contains(const Key& key) const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return find_node(key) != nullptr;
    }

    // begin()/end() salen del AVL (inorder)

    // imprime {k: v, ...} ordenado por key
    friend ostream& operator<<(ostream& os, AVLMap& m) {
        os << "{";
        bool first = true;
        m.inorder().forEach([&](const Pair& p) {
            if(!first) os << ", ";
            os << p;                     // reusa el operator<< de KVPair
            first = false;
        });
        return os << "}";
    }

    // lee {k: v, ...} y arma el mapa
    friend istream& operator>>(istream& is, AVLMap& m) {
        Token ch;
        if(!(is >> ch) || ch != '{') { is.clear(ios_base::failbit); return is; }
        while((is >> ws).peek() != '}') {            // mira el '}' sin consumirlo
            Key key; Token colon;
            if(!(is >> key >> colon) || colon != ':') break;
            string raw; Token c = '\0';
            while(is.get(c) && c != ',' && c != '}') raw += c;   // lee el valor hasta , o }
            size_t a = raw.find_first_not_of(" \t");
            size_t b = raw.find_last_not_of(" \t");
            raw = (a == string::npos) ? string() : raw.substr(a, b - a + 1);
            Value val{};
            istringstream iss(raw); iss >> val;
            m[key] = val;
            if(c == '}') return is;                  // el valor ya consumió el '}'
        }
        is >> ch;                                    // consume el '}' (caso vacío)
        return is;
    }
};

#endif // __AVLMAP_H__
