#ifndef __BTREEPAGE_H__
#define __BTREEPAGE_H__

#include <iostream>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <utility>
#include "../types.h"
#include "vector.h"
using namespace std;

enum bt_ErrorCode { bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged };

// Fwd=true forward (ascendente), Fwd=false backward (descendente), como InorderIter del BinaryTree
template<typename Trait, bool Fwd> class BTreeIterator;

template<typename Trait>
class BTreePage {
    template<typename, bool> friend class BTreeIterator;
public:
    using value_type = typename Trait::value_type;
    using Comp       = typename Trait::Comp;
    using Page       = BTreePage<Trait>;
    using KeyNode    = VectorNode<value_type>;     // nodo (clave, ref)
    using KeyVec     = Vector<VectorTrait<value_type>>;
    using ChildVec   = Vector<VectorTrait<Page*>>;

private:
    KeyVec   m_keys;
    ChildVec m_children;        // siempre m_keys.size()+1; nullptr = posicion de hoja
    Comp     m_comp;
    size_t   m_maxKeys;         // capacidad de esta pagina (raiz: 2*orden+1, resto: orden)
    size_t   m_maxKeysForChilds;// capacidad con la que crea hijos (distingue la raiz)
    Page*    m_parent = nullptr;// padre (raiz = nullptr); lo usa el iterador para subir

    // refija el padre de cada hijo no nulo a esta pagina
    void reparentChildren() {
        for(size_t i = 0; i < m_children.size(); ++i)
            if(Page* c = m_children[i].getData()) c->m_parent = this;
    }

    // ── capacidad ────────────────────────────────────────────────────────────
    size_t minKeys()   { return 2 * m_maxKeys / 3; }
    size_t freeCells() { return m_maxKeys - m_keys.size(); }
    size_t keyCount()  { return m_keys.size(); }
    bool   underflow() { return m_keys.size() < minKeys(); }
    bool   isRoot()    { return m_maxKeysForChilds != m_maxKeys; }

    size_t freeCellsOnLeft(size_t pos)  { return pos > 0 ? m_children[pos-1].getData()->freeCells() : 0; }
    size_t freeCellsOnRight(size_t pos) { return pos < m_keys.size() ? m_children[pos+1].getData()->freeCells() : 0; }

    // ── busqueda en la pagina ────────────────────────────────────────────────
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
    bool equals(const value_type& a, const value_type& b) { return !m_comp(a, b) && !m_comp(b, a); }

    // ── helpers de insercion/borrado en posicion (push_back + swap, sin STL) ──
    void insertKeyAt(size_t pos, const value_type& key, Ref ref) {
        m_keys.push_back(key, ref);
        for(size_t j = m_keys.size() - 1; j > pos; --j) swap(m_keys[j], m_keys[j-1]);
    }
    void insertChildAt(size_t pos, Page* child) {
        m_children.push_back(child, Ref{});
        for(size_t j = m_children.size() - 1; j > pos; --j) swap(m_children[j], m_children[j-1]);
    }
    void removeKeyAt(size_t pos) {
        for(size_t j = pos; j + 1 < m_keys.size(); ++j) swap(m_keys[j], m_keys[j+1]);
        m_keys.pop_back();
    }
    void removeChildAt(size_t pos) {
        for(size_t j = pos; j + 1 < m_children.size(); ++j) swap(m_children[j], m_children[j+1]);
        m_children.pop_back();
    }

    // vacia los vectores SIN liberar los hijos (se reubican en un split)
    void resetEmpty() {
        m_keys     = KeyVec(m_maxKeys + 1);
        m_children = ChildVec(m_maxKeys + 2);
    }

    // ── B*: redistribuir ──────────────────────────────────────────────────────
    // dos ramas: underflow (Remove, trae de un hermano) y overflow (Insert, cede a un hermano)
    bool redistribute1(size_t& pos) {
        Page* child = m_children[pos].getData();
        if(child->underflow()) {                             // Remove: el hijo quedo corto
            size_t nkol = (pos > 0)             ? m_children[pos-1].getData()->keyCount() : 0;
            size_t nkor = (pos < m_keys.size()) ? m_children[pos+1].getData()->keyCount() : 0;
            if(nkol > nkor) {
                if(m_children[pos-1].getData()->keyCount() > m_children[pos-1].getData()->minKeys())
                    redistributeL2R(pos-1);                  // trae del hermano izquierdo
                else if(pos == m_keys.size()) { --pos; return false; }
                else return false;
            } else {
                if(m_children[pos+1].getData()->keyCount() > m_children[pos+1].getData()->minKeys())
                    redistributeR2L(pos+1);                  // trae del hermano derecho
                else if(pos == 0) { ++pos; return false; }
                else return false;
            }
        } else {                                             // Insert: el hijo se desbordo
            size_t fcol = freeCellsOnLeft(pos);
            size_t fcor = freeCellsOnRight(pos);
            if(fcol == 0 && fcor == 0 && child->keyCount() >= child->m_maxKeys) return false;
            if(fcol > fcor) redistributeR2L(pos);
            else            redistributeL2R(pos);
        }
        return true;
    }

    // dos hermanos underflow: rota para arreglarlos; si falla, hay que hacer merge
    bool redistribute2(size_t pos) {
        if(m_children[pos-1].getData()->underflow()) {
            redistributeR2L(pos+1);
            redistributeR2L(pos);
            if(m_children[pos-1].getData()->underflow()) return false;
        } else if(m_children[pos+1].getData()->underflow()) {
            redistributeL2R(pos-1);
            redistributeL2R(pos);
            if(m_children[pos+1].getData()->underflow()) return false;
        } else {
            redistributeL2R(pos-1);
            redistributeR2L(pos+1);
            if(m_children[pos].getData()->underflow()) return false;
        }
        return true;
    }

    bool treatUnderflow(size_t& pos) { return redistribute1(pos) || redistribute2(pos); }

    // mueve del hermano derecho (pos) al izquierdo (pos-1) rotando por la clave del padre
    void redistributeR2L(size_t pos) {
        Page* source = m_children[pos].getData();
        Page* target = m_children[pos-1].getData();
        while(source->keyCount() > source->minKeys() && target->keyCount() < source->keyCount()) {
            target->m_keys.push_back(m_keys[pos-1].getData(), m_keys[pos-1].getRef());      // clave del padre baja al final
            target->m_children.push_back(source->m_children[0].getData(), Ref{});           // hijo izq de source al final
            m_keys[pos-1].setData(source->m_keys[0].getData());                              // clave izq de source sube
            m_keys[pos-1].setRef(source->m_keys[0].getRef());
            source->removeKeyAt(0);
            source->removeChildAt(0);
        }
        target->reparentChildren();   // el hijo movido cambio de dueño
    }
    // mueve del hermano izquierdo (pos) al derecho (pos+1) rotando por la clave del padre
    void redistributeL2R(size_t pos) {
        Page* source = m_children[pos].getData();
        Page* target = m_children[pos+1].getData();
        while(source->keyCount() > source->minKeys() && target->keyCount() < source->keyCount()) {
            target->insertKeyAt(0, m_keys[pos].getData(), m_keys[pos].getRef());             // clave del padre baja al frente
            target->insertChildAt(0, source->m_children[source->m_children.size()-1].getData()); // hijo der de source al frente
            m_keys[pos].setData(source->m_keys[source->m_keys.size()-1].getData());          // clave der de source sube
            m_keys[pos].setRef(source->m_keys[source->m_keys.size()-1].getRef());
            source->m_keys.pop_back();
            source->m_children.pop_back();
        }
        target->reparentChildren();   // el hijo movido cambio de dueño
    }

    // ── B*: split 2 paginas llenas -> 3 ──────────────────────────────────────
    // vuelca claves+hijos de un hijo a los temporales y lo deja vacio (sin liberar los hijos)
    void movePage(Page* child, KeyVec& tk, ChildVec& tc) {
        size_t n = child->m_keys.size();
        for(size_t i = 0; i < n; ++i) {
            tk.push_back(child->m_keys[i].getData(), child->m_keys[i].getRef());
            tc.push_back(child->m_children[i].getData(), Ref{});
        }
        tc.push_back(child->m_children[n].getData(), Ref{});
        child->resetEmpty();   // vacio: sus hijos ya viven en tc, no los libera
    }

    // reparte los temporales (2 paginas + 1 separador) en 3 paginas; sube oi1, oi2
    void splitPageInto3(KeyVec& tk, ChildVec& tc, Page*& c1, Page*& c2, Page*& c3, KeyNode& oi1, KeyNode& oi2) {
        size_t total = tk.size();
        size_t third = (total - 2) / 3;
        if(!c1) c1 = new Page(m_maxKeysForChilds);
        c1->resetEmpty();
        size_t i = 0;
        for(; i < third; ++i) {
            c1->m_keys.push_back(tk[i].getData(), tk[i].getRef());
            c1->m_children.push_back(tc[i].getData(), Ref{});
        }
        c1->m_children.push_back(tc[i].getData(), Ref{});
        oi1.setData(tk[i].getData()); oi1.setRef(tk[i].getRef()); ++i;

        if(!c2) c2 = new Page(m_maxKeysForChilds);
        c2->resetEmpty();
        size_t limit = 2 * third + 1;
        for(; i < limit; ++i) {
            c2->m_keys.push_back(tk[i].getData(), tk[i].getRef());
            c2->m_children.push_back(tc[i].getData(), Ref{});
        }
        c2->m_children.push_back(tc[i].getData(), Ref{});
        oi2.setData(tk[i].getData()); oi2.setRef(tk[i].getRef()); ++i;

        if(!c3) c3 = new Page(m_maxKeysForChilds);
        c3->resetEmpty();
        for(; i < total; ++i) {
            c3->m_keys.push_back(tk[i].getData(), tk[i].getRef());
            c3->m_children.push_back(tc[i].getData(), Ref{});
        }
        c3->m_children.push_back(tc[i].getData(), Ref{});
        c1->reparentChildren(); c2->reparentChildren(); c3->reparentChildren();
    }

    // empareja el hijo lleno m_children[pos] con un hermano lleno y los divide 2->3
    void splitChild(size_t pos) {
        Page* c1 = nullptr; Page* c2 = nullptr;
        if(pos > 0 && isFullChild(pos-1)) { c1 = m_children[pos-1].getData(); c2 = m_children[pos].getData(); --pos; }
        if(pos < m_keys.size() && isFullChild(pos+1)) { c1 = m_children[pos].getData(); c2 = m_children[pos+1].getData(); }

        KeyVec   tk(2 * m_maxKeysForChilds + 2);
        ChildVec tc(2 * m_maxKeysForChilds + 3);
        movePage(c1, tk, tc);
        tk.push_back(m_keys[pos].getData(), m_keys[pos].getRef());   // separador
        movePage(c2, tk, tc);

        Page* c3 = nullptr;
        KeyNode oi1, oi2;
        splitPageInto3(tk, tc, c1, c2, c3, oi1, oi2);

        m_keys[pos].setData(oi1.getData()); m_keys[pos].setRef(oi1.getRef());
        m_children[pos].setData(c1);
        insertKeyAt(pos + 1, oi2.getData(), oi2.getRef());
        insertChildAt(pos + 1, c2);
        m_children[pos + 2].setData(c3);
        reparentChildren();
    }

    bool isFullChild(size_t i) { Page* c = m_children[i].getData(); return c->keyCount() >= c->m_maxKeys; }

    // ── B*: merge (borrado) ───────────────────────────────────────────────────
    // fusiona 3 hijos (pos-1, pos, pos+1) + 2 separadores en 2 paginas; el padre pierde 1 clave
    bt_ErrorCode merge(size_t pos) {
        KeyVec   tk(3 * m_maxKeysForChilds + 2);
        ChildVec tc(3 * m_maxKeysForChilds + 3);
        Page* c1 = m_children[pos-1].getData();
        Page* c2 = m_children[pos].getData();
        Page* c3 = m_children[pos+1].getData();
        movePage(c1, tk, tc);
        tk.push_back(m_keys[pos-1].getData(), m_keys[pos-1].getRef());
        movePage(c2, tk, tc);
        tk.push_back(m_keys[pos].getData(), m_keys[pos].getRef());
        movePage(c3, tk, tc);
        delete c3;                                       // c3 quedo vacio -> no libera nada

        size_t n1 = c1->m_maxKeys;                       // c1 vacio -> freeCells = maxKeys
        size_t i = 0;
        for(; i < n1; ++i) {
            c1->m_keys.push_back(tk[i].getData(), tk[i].getRef());
            c1->m_children.push_back(tc[i].getData(), Ref{});
        }
        c1->m_children.push_back(tc[i].getData(), Ref{});
        m_keys[pos-1].setData(tk[i].getData()); m_keys[pos-1].setRef(tk[i].getRef());
        m_children[pos-1].setData(c1);
        removeKeyAt(pos);
        removeChildAt(pos);

        size_t n2 = c2->m_maxKeys;
        size_t j = i + 1;
        for(i = 0; i < n2; ++i, ++j) {
            c2->m_keys.push_back(tk[j].getData(), tk[j].getRef());
            c2->m_children.push_back(tc[j].getData(), Ref{});
        }
        c2->m_children.push_back(tc[j].getData(), Ref{});
        m_children[pos].setData(c2);

        c1->reparentChildren(); c2->reparentChildren(); reparentChildren();
        return underflow() ? bt_underflow : bt_ok;
    }

    // fusiona los 3 hijos de la raiz + 2 separadores DENTRO de la raiz (el arbol baja un nivel)
    bt_ErrorCode mergeRoot() {
        KeyVec   tk(m_maxKeys + 2);
        ChildVec tc(m_maxKeys + 3);
        Page* c1 = m_children[0].getData();
        Page* c2 = m_children[1].getData();
        Page* c3 = m_children[2].getData();
        movePage(c1, tk, tc);
        tk.push_back(m_keys[0].getData(), m_keys[0].getRef());
        movePage(c2, tk, tc);
        tk.push_back(m_keys[1].getData(), m_keys[1].getRef());
        movePage(c3, tk, tc);
        resetEmpty();
        size_t total = tk.size();
        for(size_t i = 0; i < total; ++i) {
            m_keys.push_back(tk[i].getData(), tk[i].getRef());
            m_children.push_back(tc[i].getData(), Ref{});
        }
        m_children.push_back(tc[total].getData(), Ref{});
        reparentChildren();
        delete c1; delete c2; delete c3;
        return bt_rootmerged;
    }

    // primer nodo (clave, ref) del subarbol: el mas a la izquierda (sucesor inorder)
    KeyNode& getFirstObjectInfo() {
        Page* c = m_children[0].getData();
        if(c) return c->getFirstObjectInfo();
        return m_keys[0];
    }

public:
    BTreePage(size_t maxKeys)
        : m_keys(maxKeys + 1), m_children(maxKeys + 2), m_maxKeys(maxKeys), m_maxKeysForChilds(maxKeys) {
        m_children.push_back(nullptr, Ref{});       // hoja vacia: 1 hijo nulo
    }

    BTreePage(const BTreePage&)            = delete;
    BTreePage& operator=(const BTreePage&) = delete;

    ~BTreePage() {
        for(size_t i = 0; i < m_children.size(); ++i) delete m_children[i].getData();   // delete nullptr es seguro
    }

    void   setMaxKeysForChilds(size_t m) { m_maxKeysForChilds = m; }
    bool   isLeaf() { return m_children.size() == 0 || m_children[0].getData() == nullptr; }

    // libera el subarbol y deja una hoja vacia
    void clear() {
        for(size_t i = 0; i < m_children.size(); ++i) delete m_children[i].getData();
        resetEmpty();
        m_children.push_back(nullptr, Ref{});
    }

    // insercion B*: inserta y, al volver, si el hijo se desbordo, redistribuye o parte 2->3
    bt_ErrorCode insert(const value_type& key, Ref ref) {
        size_t pos = lowerBound(key);
        if(isLeaf()) {
            insertKeyAt(pos, key, ref);
            m_children.push_back(nullptr, Ref{});   // mantiene keys+1 hijos (nulos)
            return m_keys.size() > m_maxKeys ? bt_overflow : bt_ok;
        }
        bt_ErrorCode error = m_children[pos].getData()->insert(key, ref);
        if(error == bt_overflow) {
            if(!redistribute1(pos)) splitChild(pos);
            return m_keys.size() > m_maxKeys ? bt_overflow : bt_ok;
        }
        return bt_ok;
    }

    // parte la raiz en si misma: queda con 2 claves (medianas) y 3 hijos (el arbol crece por la raiz)
    void splitRoot() {
        KeyVec   tk = move(m_keys);
        ChildVec tc = move(m_children);
        Page* c1 = nullptr; Page* c2 = nullptr; Page* c3 = nullptr;
        KeyNode oi1, oi2;
        splitPageInto3(tk, tc, c1, c2, c3, oi1, oi2);
        m_keys     = KeyVec(m_maxKeys + 1);
        m_children = ChildVec(m_maxKeys + 2);
        m_keys.push_back(oi1.getData(), oi1.getRef());
        m_keys.push_back(oi2.getData(), oi2.getRef());
        m_children.push_back(c1, Ref{});
        m_children.push_back(c2, Ref{});
        m_children.push_back(c3, Ref{});
        reparentChildren();
    }

    // busca la clave (clave, ref); lanza si no existe
    tuple<value_type, Ref> search(const value_type& key) {
        size_t pos = lowerBound(key);
        if(pos < m_keys.size() && equals(m_keys[pos].getDataRef(), key))
            return { m_keys[pos].getDataRef(), m_keys[pos].getRef() };
        Page* child = m_children[pos].getData();
        if(!child) throw runtime_error("BTree::search: clave no encontrada");
        return child->search(key);
    }

    // borrado B*: baja hasta la hoja (usando el sucesor si es interno) y trata el underflow al volver
    bt_ErrorCode remove(const value_type& key, Ref ref) {
        size_t pos = lowerBound(key);
        bt_ErrorCode error = bt_ok;
        if(pos < m_keys.size() && equals(key, m_keys[pos].getDataRef())) {
            if(!m_children[pos+1].getData()) {           // 1er caso: es hoja -> borra directo
                removeKeyAt(pos);
                removeChildAt(pos);                      // quita un hijo nulo (mantiene keys+1)
                return underflow() ? bt_underflow : bt_ok;
            }
            // 2do caso: interno -> intercambia con el sucesor y borra en la rama derecha
            KeyNode& succ = m_children[pos+1].getData()->getFirstObjectInfo();
            swap(m_keys[pos], succ);
            ++pos;
            error = m_children[pos].getData()->remove(key, ref);
        } else {                                          // no esta aqui -> baja por el hijo pos
            Page* child = m_children[pos].getData();
            if(!child) return bt_nofound;
            error = child->remove(key, ref);
        }
        if(error == bt_underflow) {                       // 3er caso: el hijo quedo corto
            if(treatUnderflow(pos)) return bt_ok;         // redistribuir con hermanos
            if(isRoot() && m_keys.size() == 2) return mergeRoot();  // 4to caso: fusionar
            return merge(pos);
        }
        if(error == bt_nofound) return bt_nofound;
        return bt_ok;
    }

    // hoja mas a la izquierda del subarbol (para el iterador)
    Page* leftmostLeaf() {
        Page* c = this;
        while(!c->isLeaf()) c = c->m_children[0].getData();
        return c;
    }
    // hoja mas a la derecha del subarbol
    Page* rightmostLeaf() {
        Page* c = this;
        while(!c->isLeaf()) c = c->m_children[c->keyCount()].getData();
        return c;
    }
};

// Iterador inorder (unico recorrido) que navega por m_parent. Fwd = direccion
template<typename Trait, bool Fwd>
class BTreeIterator {
    template<typename> friend class BTree;
public:
    using Page    = BTreePage<Trait>;
    using KeyNode = typename Page::KeyNode;

private:
    Page*  m_page  = nullptr;   // pagina actual (nullptr = fin del recorrido)
    size_t m_index = 0;         // clave actual dentro de la pagina

    BTreeIterator(Page* page, size_t index) : m_page(page), m_index(index) {}

    // indice de 'child' dentro de 'parent' (scan lineal)
    static size_t indexInParent(Page* parent, Page* child) {
        for(size_t i = 0; i < parent->m_children.size(); ++i)
            if(parent->m_children[i].getData() == child) return i;
        return parent->m_children.size();
    }

    // sucesor: baja al hijo derecho (mas izq) o sube por m_parent al ancestro pendiente
    void advanceForward() {
        Page* right = m_page->m_children[m_index + 1].getData();
        if(right) { m_page = right->leftmostLeaf(); m_index = 0; return; }
        if(++m_index < m_page->keyCount()) return;
        Page* cur = m_page;
        while(cur->m_parent) {
            size_t j = indexInParent(cur->m_parent, cur);
            if(j < cur->m_parent->keyCount()) { m_page = cur->m_parent; m_index = j; return; }
            cur = cur->m_parent;
        }
        m_page = nullptr;   // fin
    }

    // predecesor: espejo del anterior (hijo izquierdo mas der, o sube por m_parent)
    void advanceBackward() {
        Page* left = m_page->m_children[m_index].getData();
        if(left) { m_page = left->rightmostLeaf(); m_index = m_page->keyCount() - 1; return; }
        if(m_index > 0) { --m_index; return; }
        Page* cur = m_page;
        while(cur->m_parent) {
            size_t j = indexInParent(cur->m_parent, cur);
            if(j > 0) { m_page = cur->m_parent; m_index = j - 1; return; }
            cur = cur->m_parent;
        }
        m_page = nullptr;
    }

public:
    BTreeIterator() = default;   // fin (m_page == nullptr)

    // primer nodo: mas a la izquierda si Fwd, mas a la derecha si no. vacio -> fin
    static BTreeIterator first(Page* root) {
        if(root->keyCount() == 0) return BTreeIterator();
        if constexpr(Fwd) return BTreeIterator(root->leftmostLeaf(), 0);
        else { Page* leaf = root->rightmostLeaf(); return BTreeIterator(leaf, leaf->keyCount() - 1); }
    }

    KeyNode& operator*()  { return m_page->m_keys[m_index]; }
    KeyNode* operator->() { return &m_page->m_keys[m_index]; }

    BTreeIterator& operator++() {
        if constexpr(Fwd) advanceForward();
        else              advanceBackward();
        return *this;
    }
    BTreeIterator operator++(int) { BTreeIterator t = *this; ++(*this); return t; }

    // iguales si ambos son fin, o misma pagina y mismo indice
    bool operator!=(const BTreeIterator& o) const {
        if(m_page == nullptr || o.m_page == nullptr) return m_page != o.m_page;
        return m_page != o.m_page || m_index != o.m_index;
    }
    bool operator==(const BTreeIterator& o) const { return !(*this != o); }
};

#endif // __BTREEPAGE_H__
