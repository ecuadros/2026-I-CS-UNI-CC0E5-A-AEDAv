#ifndef BTREE_H
#define BTREE_H
#include <iostream>
#include <utility>
#include <cstddef>
#include "../types.h"
#include "traits.h"
#include "BTreePage.h"
using namespace std;

#define DEFAULT_BTREE_ORDER 3

template <typename Trait>
class BTree {
    typedef typename Trait::keyType   keyType;
    typedef typename Trait::ObjIDType ObjIDType;
    typedef CBTreePage<Trait>         BTNode;
public:
    typedef typename BTNode::ObjectInfo ObjectInfo;

    BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
        : m_Root(2 * order + 1, unique), m_Height(1), m_Order(order), m_NumKeys(0), m_Unique(unique) {
        m_Root.SetMaxKeysForChilds(order);
    }
    ~BTree() {}

    bool Insert(const keyType key, const ObjIDType ObjID) {
        bt_ErrorCode error = m_Root.Insert(key, ObjID);
        if (error == bt_duplicate) return false;
        m_NumKeys++;
        if (error == bt_overflow) { m_Root.SplitRoot(); m_Height++; }
        return true;
    }

    bool Remove(const keyType key, const ObjIDType ObjID) {
        bt_ErrorCode error = m_Root.Remove(key, ObjID);
        if (error == bt_duplicate || error == bt_nofound) return false;
        m_NumKeys--;
        if (error == bt_rootmerged) m_Height--;
        return true;
    }

    // devuelve el ObjID de la clave, o -1 si no esta
    ObjIDType Search(const keyType key) {
        ObjIDType ObjID = -1;
        m_Root.Search(key, ObjID);
        return ObjID;
    }

    size_t size()     { return m_NumKeys; }
    size_t height()   { return m_Height;  }
    size_t GetOrder() { return m_Order;   }

    void Print(ostream& os) { m_Root.Print(os); }

    // ForEach / FirstThat variadicos: pasan func + sus extras al recorrido de las paginas
    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        m_Root.ForEach(0, func, forward<Args>(args)...);
    }
    template <typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, Args&&... args) {
        return m_Root.FirstThat(0, func, forward<Args>(args)...);
    }

protected:
    BTNode m_Root;
    size_t m_Height, m_Order, m_NumKeys;
    bool   m_Unique;
};

#endif
