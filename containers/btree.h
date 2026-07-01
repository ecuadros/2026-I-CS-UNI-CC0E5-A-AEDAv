#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <tuple>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include "../types.h"
#include "btreepage.h"
using namespace std;

#define DEFAULT_BTREE_ORDER 3

template<typename T> struct AscendingBTreeTrait  { using value_type = T; using Comp = less<T>;    };
template<typename T> struct DescendingBTreeTrait { using value_type = T; using Comp = greater<T>; };

// Guarda la raiz (BTreePage), metadatos y el shared_mutex. delega en la raiz
template<typename Trait>
class BTree {
public:
    using value_type = typename Trait::value_type;
    using Comp       = typename Trait::Comp;
    using Page       = BTreePage<Trait>;
    using KeyNode    = typename Page::KeyNode;
    using MySelf     = BTree<Trait>;

private:
    Page   m_root;
    size_t m_order;
    size_t m_height;
    size_t m_numKeys;
    mutable shared_mutex m_mtx;

public:
    // raiz con capacidad 2*orden+1; los hijos se crean con 'orden' (B* del profe)
    BTree(size_t order = DEFAULT_BTREE_ORDER)
        : m_root(2 * order + 1), m_order(order), m_height(1), m_numKeys(0) {
        m_root.setMaxKeysForChilds(order);
    }

    BTree(const BTree&)            = delete;
    BTree& operator=(const BTree&) = delete;

    ~BTree() {
        unique_lock<shared_mutex> lock(m_mtx);
        m_root.clear();
    }

    // insercion B* (reactiva): si la raiz se desborda, la parte y sube un nivel
    void insert(const value_type& key, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        bt_ErrorCode error = m_root.insert(key, ref);
        ++m_numKeys;
        if(error == bt_overflow) { m_root.splitRoot(); ++m_height; }
    }

    // retorno (clave, ref)
    tuple<value_type, Ref> search(const value_type& key) {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_root.search(key);
    }

    // borrado B* (reactivo): si la raiz se fusiona, baja un nivel. Devuelve false si no existe
    bool remove(const value_type& key, Ref ref = Ref{}) {
        unique_lock<shared_mutex> lock(m_mtx);
        bt_ErrorCode error = m_root.remove(key, ref);
        if(error == bt_nofound) return false;
        --m_numKeys;
        if(error == bt_rootmerged) --m_height;
        return true;
    }

    // recorrido inorder (forward): callback void -> visita todo. Reusa el unico bucle (firstThat)
    template<typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_root.firstThat(func, forward<Args>(args)...);   // call maneja que func sea void
    }

    // recorrido inorder inverso (backward): mismo bucle, en reversa
    template<typename Func, typename... Args>
    void ReverseForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_root.rfirstThat(func, forward<Args>(args)...);
    }

    // primer (clave, ref) inorder que cumple el predicado; mismo bucle (firstThat), callback con valor
    template<typename Pred, typename... Args>
    tuple<value_type, Ref> FirstThat(Pred pred, Args&&... args) {
        shared_lock<shared_mutex> lock(m_mtx);
        KeyNode* p = m_root.firstThat(pred, forward<Args>(args)...);
        if(!p) throw runtime_error("BTree::FirstThat: ninguna clave cumple");
        return { p->getDataRef(), p->getRef() };
    }

    // FirstThat en reversa: primer nodo que cumple recorriendo de mayor a menor
    template<typename Pred, typename... Args>
    tuple<value_type, Ref> ReverseFirstThat(Pred pred, Args&&... args) {
        shared_lock<shared_mutex> lock(m_mtx);
        KeyNode* p = m_root.rfirstThat(pred, forward<Args>(args)...);
        if(!p) throw runtime_error("BTree::ReverseFirstThat: ninguna clave cumple");
        return { p->getDataRef(), p->getRef() };
    }

    // imprime [(clave,ref),...] recorriendo con ForEach (reusa VectorNode::operator<<)
    friend ostream& operator<<(ostream& os, BTree& bt) {
        os << "[";
        bool first = true;
        bt.ForEach([&](KeyNode& n) { os << (first ? "" : ",") << n; first = false; });
        return os << "]";
    }

    // lee [(clave,ref),...] reusando VectorNode::operator>> y reinserta (insert toma el lock)
    friend istream& operator>>(istream& is, BTree& bt) {
        Token ch;
        if(!(is >> ch) || ch != '[') { is.setstate(ios_base::failbit); return is; }
        if((is >> ws).peek() == ']') { is >> ch; return is; }
        VectorNode<value_type> node;
        while(is >> node) {
            bt.insert(node.getData(), node.getRef());
            is >> ch;
            if(ch == ']') break;
            else if(ch != ',') { is.setstate(ios_base::failbit); break; }
        }
        return is;
    }

    string toString() { ostringstream oss; oss << *this; return oss.str(); }

    size_t height() { shared_lock<shared_mutex> lock(m_mtx); return m_height;  }
    size_t size()   { shared_lock<shared_mutex> lock(m_mtx); return m_numKeys; }
    size_t order()  { shared_lock<shared_mutex> lock(m_mtx); return m_order;   }
};

#endif // __BTREE_H__
