#ifndef CBTreePage_H
#define CBTreePage_H
#include <vector>
#include <iostream>
#include <utility>
#include <cstddef>
#include "../types.h"
#include "traits.h"
using namespace std;

template <typename Trait> class BTree;

enum bt_ErrorCode { bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged };

template <typename keyType, typename ObjIDType>
struct tagObjectInfo {
    keyType   key;
    ObjIDType ObjID;
    size_t    UseCounter;
    tagObjectInfo(const keyType& _key, ObjIDType _ObjID) : key(_key), ObjID(_ObjID), UseCounter(0) {}
    tagObjectInfo() : key(keyType()), ObjID(ObjIDType()), UseCounter(0) {}
    operator keyType() const { return key; }
    size_t GetUseCounter() const { return UseCounter; }
};

// desplaza a la derecha para abrir hueco en pos
template <typename Container, typename ObjType>
void insert_at(Container& container, const ObjType& object, size_t pos) {
    for (size_t i = container.size() - 1; i > pos; i--)
        container[i] = container[i - 1];
    container[pos] = object;
}

// desplaza a la izquierda para tapar el hueco en pos
template <typename Container>
void remove(Container& container, size_t pos) {
    for (size_t i = pos + 1; i < container.size(); i++)
        container[i - 1] = container[i];
}

template <typename Trait>
class CBTreePage {
    friend class BTree<Trait>;
public:
    typedef typename Trait::keyType            keyType;
    typedef typename Trait::ObjIDType          ObjIDType;
    typedef typename Trait::Comp               Comp;
    typedef CBTreePage<Trait>                  BTPage;
    typedef tagObjectInfo<keyType, ObjIDType>  ObjectInfo;

    CBTreePage(size_t maxKeys, bool unique = true)
        : m_MaxKeys(maxKeys), m_MaxKeysForChilds(maxKeys), m_Unique(unique), m_KeyCount(0) {
        Create();
    }
    ~CBTreePage() { Reset(); }

    bt_ErrorCode Insert(const keyType& key, const ObjIDType ObjID) {
        size_t pos = binary_search(0, m_KeyCount, key);
        if (pos < m_KeyCount && SameKey(m_Keys[pos].key, key) && m_Unique)
            return bt_duplicate;
        if (!m_SubPages[pos]) {                       // es hoja: insertar aqui
            ::insert_at(m_Keys, ObjectInfo(key, ObjID), pos);
            m_KeyCount++;
            return Overflow() ? bt_overflow : bt_ok;
        }
        bt_ErrorCode error = m_SubPages[pos]->Insert(key, ObjID);
        if (error == bt_duplicate) return bt_duplicate;
        if (error == bt_overflow)
            if (!Redistribute1(pos)) SplitChild(pos);
        return Overflow() ? bt_overflow : bt_ok;
    }

    bt_ErrorCode Remove(const keyType& key, const ObjIDType ObjID) {
        bt_ErrorCode error = bt_ok;
        size_t pos = binary_search(0, m_KeyCount, key);
        if (pos < m_KeyCount && SameKey(key, m_Keys[pos].key)) {
            if (!m_SubPages[pos + 1]) {               // la encontramos en una hoja
                ::remove(m_Keys, pos);
                m_KeyCount--;
                return Underflow() ? bt_underflow : bt_ok;
            }
            // nodo interno: la cambiamos por el primero de la rama derecha y borramos abajo
            ObjectInfo& rFirst = m_SubPages[pos + 1]->GetFirstObjectInfo();
            swap(m_Keys[pos], rFirst);
            error = m_SubPages[++pos]->Remove(key, ObjID);
        } else {
            if (!m_SubPages[pos]) return bt_nofound;  // no existe
            error = m_SubPages[pos]->Remove(key, ObjID);
        }
        if (error == bt_underflow) {
            if (TreatUnderflow(pos)) return bt_ok;
            if (IsRoot() && m_KeyCount == 2) return MergeRoot();
            return Merge(pos);
        }
        return error;
    }

    bool Search(const keyType& key, ObjIDType& ObjID) {
        size_t pos = binary_search(0, m_KeyCount, key);
        if (pos < m_KeyCount && SameKey(key, m_Keys[pos].key)) {
            ObjID = m_Keys[pos].ObjID;
            m_Keys[pos].UseCounter++;
            return true;
        }
        return m_SubPages[pos] ? m_SubPages[pos]->Search(key, ObjID) : false;
    }

    void Print(ostream& os) {
        ForEach(0, [](ObjectInfo& info, size_t level, ostream& os) {
            for (size_t i = 0; i < level; i++) os << "\t";
            os << info.key << "->" << info.ObjID << "\n";
        }, os);
    }

    // recorre inorder; func recibe (ObjectInfo&, nivel, ...extras)
    template <typename Func, typename... Args>
    void ForEach(size_t level, Func func, Args&&... args) {
        for (size_t i = 0; i < m_KeyCount; i++) {
            if (m_SubPages[i]) m_SubPages[i]->ForEach(level + 1, func, forward<Args>(args)...);
            func(m_Keys[i], level, forward<Args>(args)...);
        }
        if (m_SubPages[m_KeyCount]) m_SubPages[m_KeyCount]->ForEach(level + 1, func, forward<Args>(args)...);
    }

    // devuelve el primer ObjectInfo (inorder) que cumple func, o 0
    template <typename Func, typename... Args>
    ObjectInfo* FirstThat(size_t level, Func func, Args&&... args) {
        for (size_t i = 0; i < m_KeyCount; i++) {
            if (m_SubPages[i]) {
                ObjectInfo* found = m_SubPages[i]->FirstThat(level + 1, func, forward<Args>(args)...);
                if (found) return found;
            }
            if (func(m_Keys[i], level, forward<Args>(args)...)) return &m_Keys[i];
        }
        if (m_SubPages[m_KeyCount])
            return m_SubPages[m_KeyCount]->FirstThat(level + 1, func, forward<Args>(args)...);
        return 0;
    }

protected:
    Comp   m_comp;
    size_t m_MinKeys, m_MaxKeys, m_MaxKeysForChilds;
    bool   m_Unique;
    vector<ObjectInfo> m_Keys;
    vector<BTPage*>    m_SubPages;
    size_t m_KeyCount;

    bool SameKey(const keyType& a, const keyType& b) { return !m_comp(a, b) && !m_comp(b, a); }

    // primera posicion cuya clave no va antes que key (segun el comparador)
    size_t binary_search(size_t first, size_t last, const keyType& key) {
        while (first < last) {
            size_t mid = (first + last) / 2;
            if (m_comp(m_Keys[mid].key, key)) first = mid + 1;
            else                              last  = mid;
        }
        return first;
    }

    bool   Overflow()        { return m_KeyCount > m_MaxKeys; }
    bool   Underflow()       { return m_KeyCount < MinNumberOfKeys(); }
    bool   IsFull()          { return m_KeyCount >= m_MaxKeys; }
    size_t MinNumberOfKeys() { return 2 * m_MaxKeys / 3; }
    size_t GetFreeCells()    { return m_MaxKeys - m_KeyCount; }
    bool   IsRoot()          { return m_MaxKeysForChilds != m_MaxKeys; }
    void   SetMaxKeysForChilds(size_t n) { m_MaxKeysForChilds = n; }
    size_t GetFreeCellsOnLeft(size_t pos)  { return pos > 0          ? m_SubPages[pos - 1]->GetFreeCells() : 0; }
    size_t GetFreeCellsOnRight(size_t pos) { return pos < m_KeyCount ? m_SubPages[pos + 1]->GetFreeCells() : 0; }

    void Create() {
        m_Keys.resize(m_MaxKeys + 1);
        m_SubPages.resize(m_MaxKeys + 2, NULL);
        m_KeyCount = 0;
        m_MinKeys  = 2 * m_MaxKeys / 3;
    }
    void clear() { m_KeyCount = 0; }
    void Reset() {
        for (size_t i = 0; i <= m_KeyCount; i++) { delete m_SubPages[i]; m_SubPages[i] = NULL; }
        clear();
    }
    void Destroy() { Reset(); delete this; }

    ObjectInfo& GetFirstObjectInfo() {
        return m_SubPages[0] ? m_SubPages[0]->GetFirstObjectInfo() : m_Keys[0];
    }

    // vuelca esta pagina en los vectores temporales y suelta sus hijos (los deja en NULL)
    void MovePage(BTPage* child, vector<ObjectInfo>& tmpKeys, vector<BTPage*>& tmpSubPages) {
        size_t i = 0;
        for (; i < child->m_KeyCount; i++) {
            tmpKeys.push_back(child->m_Keys[i]);
            tmpSubPages.push_back(child->m_SubPages[i]);
            child->m_SubPages[i] = NULL;
        }
        tmpSubPages.push_back(child->m_SubPages[i]);
        child->m_SubPages[i] = NULL;
        child->clear();
    }

    bool Redistribute1(size_t& pos) {
        if (m_SubPages[pos]->Underflow()) {
            size_t nkol = pos > 0          ? m_SubPages[pos - 1]->m_KeyCount : 0;
            size_t nkor = pos < m_KeyCount ? m_SubPages[pos + 1]->m_KeyCount : 0;
            if (nkol > nkor) {
                if (m_SubPages[pos - 1]->m_KeyCount > m_SubPages[pos - 1]->MinNumberOfKeys())
                    RedistributeL2R(pos - 1);
                else if (pos == m_KeyCount) return (pos--, false);
                else return false;
            } else {
                if (m_SubPages[pos + 1]->m_KeyCount > m_SubPages[pos + 1]->MinNumberOfKeys())
                    RedistributeR2L(pos + 1);
                else if (pos == 0) return (pos++, false);
                else return false;
            }
        } else {                                      // viene de un overflow
            size_t fcol = GetFreeCellsOnLeft(pos), fcor = GetFreeCellsOnRight(pos);
            if (!fcol && !fcor && m_SubPages[pos]->IsFull()) return false;
            if (fcol > fcor) RedistributeR2L(pos);
            else             RedistributeL2R(pos);
        }
        return true;
    }

    bool Redistribute2(size_t pos) {
        if (m_SubPages[pos - 1]->Underflow()) {
            RedistributeR2L(pos + 1);
            RedistributeR2L(pos);
            if (m_SubPages[pos - 1]->Underflow()) return false;
        } else if (m_SubPages[pos + 1]->Underflow()) {
            RedistributeL2R(pos - 1);
            RedistributeL2R(pos);
            if (m_SubPages[pos + 1]->Underflow()) return false;
        } else {
            RedistributeL2R(pos - 1);
            RedistributeR2L(pos + 1);
            if (m_SubPages[pos]->Underflow()) return false;
        }
        return true;
    }

    bool TreatUnderflow(size_t& pos) { return Redistribute1(pos) || Redistribute2(pos); }

    void RedistributeR2L(size_t pos) {
        BTPage* pSource = m_SubPages[pos];
        BTPage* pTarget = m_SubPages[pos - 1];
        while (pSource->m_KeyCount > pSource->MinNumberOfKeys() && pTarget->m_KeyCount < pSource->m_KeyCount) {
            ::insert_at(pTarget->m_Keys, m_Keys[pos - 1], pTarget->m_KeyCount++);
            ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[0], pTarget->m_KeyCount);
            m_Keys[pos - 1] = pSource->m_Keys[0];
            ::remove(pSource->m_Keys, 0);
            ::remove(pSource->m_SubPages, 0);
            pSource->m_KeyCount--;
        }
    }

    void RedistributeL2R(size_t pos) {
        BTPage* pSource = m_SubPages[pos];
        BTPage* pTarget = m_SubPages[pos + 1];
        while (pSource->m_KeyCount > pSource->MinNumberOfKeys() && pTarget->m_KeyCount < pSource->m_KeyCount) {
            ::insert_at(pTarget->m_Keys, m_Keys[pos], 0);
            ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[pSource->m_KeyCount], 0);
            pTarget->m_KeyCount++;
            m_Keys[pos] = pSource->m_Keys[pSource->m_KeyCount - 1];
            pSource->m_KeyCount--;
        }
    }

    void SplitChild(size_t pos) {
        BTPage *pChild1 = 0, *pChild2 = 0;
        if (pos > 0 && m_SubPages[pos - 1]->IsFull())          { pChild1 = m_SubPages[pos - 1]; pChild2 = m_SubPages[pos]; pos--; }
        if (pos < m_KeyCount && m_SubPages[pos + 1]->IsFull()) { pChild1 = m_SubPages[pos];     pChild2 = m_SubPages[pos + 1]; }

        vector<ObjectInfo> tmpKeys;
        vector<BTPage*>    tmpSubPages;
        MovePage(pChild1, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos]);
        MovePage(pChild2, tmpKeys, tmpSubPages);

        BTPage* pChild3 = 0;
        ObjectInfo oi1, oi2;
        SplitPageInto3(tmpKeys, tmpSubPages, pChild1, pChild2, pChild3, oi1, oi2);

        m_Keys[pos] = oi1; m_SubPages[pos] = pChild1;
        ::insert_at(m_Keys, oi2, pos + 1);
        ::insert_at(m_SubPages, pChild2, pos + 1);
        m_KeyCount++;
        m_SubPages[pos + 2] = pChild3;
    }

    bool SplitRoot() {
        BTPage *pChild1 = 0, *pChild2 = 0, *pChild3 = 0;
        ObjectInfo oi1, oi2;
        SplitPageInto3(m_Keys, m_SubPages, pChild1, pChild2, pChild3, oi1, oi2);
        clear();
        m_Keys[0] = oi1; m_SubPages[0] = pChild1; m_KeyCount++;
        m_Keys[1] = oi2; m_SubPages[1] = pChild2; m_KeyCount++;
        m_SubPages[2] = pChild3;
        return true;
    }

    // reparte los temporales en 3 paginas, subiendo 2 separadores (oi1, oi2)
    void SplitPageInto3(vector<ObjectInfo>& tmpKeys, vector<BTPage*>& tmpSubPages,
                        BTPage*& pChild1, BTPage*& pChild2, BTPage*& pChild3,
                        ObjectInfo& oi1, ObjectInfo& oi2) {
        size_t n = tmpKeys.size();
        size_t nKeys = (n - 2) / 3;

        if (!pChild1) pChild1 = new BTPage(m_MaxKeysForChilds, m_Unique);
        pChild1->clear();
        size_t i = 0;
        for (; i < nKeys; i++) { pChild1->m_Keys[i] = tmpKeys[i]; pChild1->m_SubPages[i] = tmpSubPages[i]; pChild1->m_KeyCount++; }
        pChild1->m_SubPages[i] = tmpSubPages[i];
        oi1 = tmpKeys[i++];

        if (!pChild2) pChild2 = new BTPage(m_MaxKeysForChilds, m_Unique);
        pChild2->clear();
        size_t bound = 2 * nKeys + 1;
        for (size_t j = 0; i < bound; i++, j++) { pChild2->m_Keys[j] = tmpKeys[i]; pChild2->m_SubPages[j] = tmpSubPages[i]; pChild2->m_KeyCount++; }
        pChild2->m_SubPages[pChild2->m_KeyCount] = tmpSubPages[i];
        oi2 = tmpKeys[i++];

        if (!pChild3) pChild3 = new BTPage(m_MaxKeysForChilds, m_Unique);
        pChild3->clear();
        for (size_t j = 0; i < n; i++, j++) { pChild3->m_Keys[j] = tmpKeys[i]; pChild3->m_SubPages[j] = tmpSubPages[i]; pChild3->m_KeyCount++; }
        pChild3->m_SubPages[pChild3->m_KeyCount] = tmpSubPages[i];
    }

    // fusiona los hijos pos-1, pos, pos+1 (con 2 separadores) en 2 paginas; el padre pierde 1 clave
    bt_ErrorCode Merge(size_t pos) {
        vector<ObjectInfo> tmpKeys;
        vector<BTPage*>    tmpSubPages;
        BTPage *pChild1 = m_SubPages[pos - 1], *pChild2 = m_SubPages[pos], *pChild3 = m_SubPages[pos + 1];
        MovePage(pChild1, tmpKeys, tmpSubPages); tmpKeys.push_back(m_Keys[pos - 1]);
        MovePage(pChild2, tmpKeys, tmpSubPages); tmpKeys.push_back(m_Keys[pos]);
        MovePage(pChild3, tmpKeys, tmpSubPages);
        pChild3->Destroy();

        size_t total = tmpKeys.size();
        size_t nKeys = (total - 1) / 2, i = 0;    // reparto parejo entre las 2 paginas
        for (; i < nKeys; i++) { pChild1->m_Keys[i] = tmpKeys[i]; pChild1->m_SubPages[i] = tmpSubPages[i]; pChild1->m_KeyCount++; }
        pChild1->m_SubPages[i] = tmpSubPages[i];
        m_Keys[pos - 1] = tmpKeys[i]; m_SubPages[pos - 1] = pChild1;
        ::remove(m_Keys, pos);
        ::remove(m_SubPages, pos);
        m_KeyCount--;

        size_t rest = total - 1 - nKeys, j = ++i;
        for (i = 0; i < rest; i++, j++) { pChild2->m_Keys[i] = tmpKeys[j]; pChild2->m_SubPages[i] = tmpSubPages[j]; pChild2->m_KeyCount++; }
        pChild2->m_SubPages[i] = tmpSubPages[j];
        m_SubPages[pos] = pChild2;

        return Underflow() ? bt_underflow : bt_ok;
    }

    // la raiz absorbe sus 3 hijos y baja una altura
    bt_ErrorCode MergeRoot() {
        BTPage *pChild1 = m_SubPages[0], *pChild2 = m_SubPages[1], *pChild3 = m_SubPages[2];
        size_t total = pChild1->m_KeyCount + pChild2->m_KeyCount + pChild3->m_KeyCount + 2;

        vector<ObjectInfo> tmpKeys;
        vector<BTPage*>    tmpSubPages;
        MovePage(pChild1, tmpKeys, tmpSubPages); tmpKeys.push_back(m_Keys[0]);
        MovePage(pChild2, tmpKeys, tmpSubPages); tmpKeys.push_back(m_Keys[1]);
        MovePage(pChild3, tmpKeys, tmpSubPages);

        clear();
        size_t i = 0;
        for (; i < total; i++) { m_Keys[i] = tmpKeys[i]; m_SubPages[i] = tmpSubPages[i]; m_KeyCount++; }
        m_SubPages[i] = tmpSubPages[i];

        pChild1->Destroy(); pChild2->Destroy(); pChild3->Destroy();
        return bt_rootmerged;
    }
};

#endif
