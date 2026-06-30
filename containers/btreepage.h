#ifndef __BTREEPAGE_H__
#define __BTREEPAGE_H__

#include <iostream>
#include <sstream>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <utility>
#include "../types.h"
#include "vector.h"
using namespace std;

// Pagina (nodo) de un B-Tree: varias claves ordenadas + sus hijos
template<typename Trait>
class BTreePage {
public:
    using value_type = typename Trait::value_type;
    using Comp       = typename Trait::Comp;
    using Page       = BTreePage<Trait>;
    using KeyNode    = VectorNode<value_type>;     // nodo (clave, ref)

private:
    Vector<VectorTrait<value_type>> m_keys;       // claves (clave = m_data, ref = m_ref)
    Vector<VectorTrait<Page*>>      m_children;    // hijos (claves + 1 si es interna)
    Comp   m_comp;
    size_t m_order;     // grado minimo t: cada pagina tiene t-1 .. 2t-1 claves
    bool   m_leaf;

    // primera posicion i con m_keys[i] >= key
    size_t lowerBound(const value_type& key) {
        size_t lo = 0, hi = m_keys.size();
        while(lo < hi) {
            size_t mid = (lo + hi) / 2;
            if(m_comp(m_keys[mid].getDataRef(), key)) lo = mid + 1;
            else                                      hi = mid;
        }
        return lo;
    }

    bool equals(const value_type& a, const value_type& b) {
        return !m_comp(a, b) && !m_comp(b, a);
    }

    // inserta en una posicion exacta: push_back + subir con swap
    void insertKeyAt(size_t pos, const value_type& key, Ref ref) {
        m_keys.push_back(key, ref);
        for(size_t j = m_keys.size() - 1; j > pos; --j)
            swap(m_keys[j], m_keys[j - 1]);
    }
    void insertChildAt(size_t pos, Page* child) {
        m_children.push_back(child, Ref{});
        for(size_t j = m_children.size() - 1; j > pos; --j)
            swap(m_children[j], m_children[j - 1]);
    }

    // divide el hijo lleno m_children[i] en dos de t-1 claves. la mediana sube a la posicion i
    void splitChild(size_t i) {
        size_t t = m_order;
        Page*  child = m_children[i].getData();
        Page*  z = new Page(child->m_leaf, t);

        // z recibe la mitad superior: claves [t..2t-2]. si es interno: hijos [t..2t-1]
        for(size_t j = t; j < 2 * t - 1; ++j)
            z->m_keys.push_back(child->m_keys[j].getDataRef(), child->m_keys[j].getRef());
        if(!child->m_leaf)
            for(size_t j = t; j < 2 * t; ++j)
                z->m_children.push_back(child->m_children[j].getData(), Ref{});

        // mediana que sube
        value_type medKey = child->m_keys[t - 1].getDataRef();
        Ref        medRef = child->m_keys[t - 1].getRef();

        // recorta el hijo a t-1 claves. t hijos si es interno
        while(child->m_keys.size() > t - 1) child->m_keys.pop_back();
        if(!child->m_leaf)
            while(child->m_children.size() > t) child->m_children.pop_back();

        // sube la mediana y engancha z como hijo derecho
        insertKeyAt(i, medKey, medRef);
        insertChildAt(i + 1, z);
    }

public:
    BTreePage(bool leaf, size_t order)
        : m_keys(2 * order), m_children(2 * order + 1), m_order(order), m_leaf(leaf) {}

    BTreePage(const BTreePage&)            = delete;
    BTreePage& operator=(const BTreePage&) = delete;

    ~BTreePage() {
        for(size_t i = 0; i < m_children.size(); ++i)
            delete m_children[i].getData();
    }

    bool   isFull() { return m_keys.size() == 2 * m_order - 1; }

    // libera el subarbol y deja la pagina como hoja vacia
    void clear() {
        for(size_t i = 0; i < m_children.size(); ++i)
            delete m_children[i].getData();
        m_children = Vector<VectorTrait<Page*>>(2 * m_order + 1);
        m_keys     = Vector<VectorTrait<value_type>>(2 * m_order);
        m_leaf     = true;
    }

    // inserta en una pagina que NO esta llena (desde arriba)
    void insertNonFull(const value_type& key, Ref ref) {
        if(m_leaf) {
            insertKeyAt(lowerBound(key), key, ref);
            return;
        }
        size_t i = lowerBound(key);
        Page*  child = m_children[i].getData();
        if(child->isFull()) {
            splitChild(i);
            // tras el split la mediana quedo en m_keys[i]. si la clave es mayor, baja a la derecha
            if(m_comp(m_keys[i].getDataRef(), key)) ++i;
            child = m_children[i].getData();
        }
        child->insertNonFull(key, ref);
    }

    // divide la raiz llena en si misma: queda con la mediana y 2 hijos (el arbol crece por la raiz)
    void splitRoot() {
        size_t t = m_order;
        Page*  left  = new Page(m_leaf, t);
        Page*  right = new Page(m_leaf, t);

        for(size_t j = 0;     j < t - 1;     ++j)
            left ->m_keys.push_back(m_keys[j].getDataRef(), m_keys[j].getRef());
        for(size_t j = t;     j < 2 * t - 1; ++j)
            right->m_keys.push_back(m_keys[j].getDataRef(), m_keys[j].getRef());
        if(!m_leaf) {
            for(size_t j = 0; j < t;     ++j)
                left ->m_children.push_back(m_children[j].getData(), Ref{});
            for(size_t j = t; j < 2 * t; ++j)
                right->m_children.push_back(m_children[j].getData(), Ref{});
        }

        value_type medKey = m_keys[t - 1].getDataRef();
        Ref        medRef = m_keys[t - 1].getRef();

        // reinicia como raiz interna (los hijos viejos ya quedaron repartidos en left/right)
        m_keys     = Vector<VectorTrait<value_type>>(2 * t);
        m_children = Vector<VectorTrait<Page*>>(2 * t + 1);
        m_leaf     = false;
        m_keys.push_back(medKey, medRef);
        m_children.push_back(left,  Ref{});
        m_children.push_back(right, Ref{});
    }

    // busca la clave (clave, ref)
    tuple<value_type, Ref> search(const value_type& key) {
        size_t i = lowerBound(key);
        if(i < m_keys.size() && equals(m_keys[i].getDataRef(), key))
            return { m_keys[i].getDataRef(), m_keys[i].getRef() };
        if(m_leaf) throw runtime_error("BTree::search: clave no encontrada");
        return m_children[i].getData()->search(key);
    }

    // recorrido inorder: hijo, clave, hijo, clave, ... ultimo hijo. pasa el nodo (clave+ref)
    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        size_t n = m_keys.size();
        for(size_t i = 0; i < n; ++i) {
            if(!m_leaf) m_children[i].getData()->forEach(func, args...);
            func(m_keys[i], args...);
        }
        if(!m_leaf) m_children[n].getData()->forEach(func, args...);
    }

    // primer nodo (clave, ref) inorder que cumple el predicado
    template<typename Pred, typename... Args>
    KeyNode* firstThat(Pred pred, Args&&... args) {
        size_t n = m_keys.size();
        for(size_t i = 0; i < n; ++i) {
            if(!m_leaf) {
                KeyNode* r = m_children[i].getData()->firstThat(pred, args...);
                if(r) return r;
            }
            if(pred(m_keys[i], args...))
                return &m_keys[i];
        }
        if(!m_leaf) {
            KeyNode* r = m_children[n].getData()->firstThat(pred, args...);
            if(r) return r;
        }
        return nullptr;
    }
};

#endif // __BTREEPAGE_H__
