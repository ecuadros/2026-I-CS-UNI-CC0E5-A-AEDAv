// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <sstream>
#include <mutex>
#include <shared_mutex>
#include "BTreePage.h"
#include "traits.h"
#include "general_iterator.h"
#include "../types.h"

#define DEFAULT_BTREE_ORDER 3

template <typename Trait> class btree_forward_iterator;
template <typename Trait> class btree_backward_iterator;

template <typename Trait>
class BTree 
// this is the full version of the BTree
{
       typedef BTreePage <Trait> BTNode;

       using keyType    = typename Trait::value_type;
       using ObjIDType  = typename Trait::objIdType;
       using Comp       = typename Trait::Comp;

public:
         typedef typename BTNode::ObjectInfo      ObjectInfo;
         typedef ObjectInfo                       Node;
         typedef ObjectInfo                       value_type;

         using forward_iterator  = btree_forward_iterator<Trait>;
         friend forward_iterator;
         using backward_iterator = btree_backward_iterator<Trait>;
         friend backward_iterator;

public:
        BTree(T1 order = DEFAULT_BTREE_ORDER, TBool unique = true);
        ~BTree();
        TBool            Insert (const keyType key, const ObjIDType ObjID);
        TBool            Remove (const keyType key, const ObjIDType ObjID);
        ObjIDType       Search (const keyType key);
        TLong            size()  { shared_lock<shared_mutex> lock(m_mtx); return m_NumKeys; }
        T1            height() { shared_lock<shared_mutex> lock(m_mtx); return m_Height; }
        T1            GetOrder() { shared_lock<shared_mutex> lock(m_mtx); return m_Order; }

        void            Print (ostream &os);
        template <typename Func, typename... Args>
        void            ForEach(Func&& lpfn, Args&&... args);
        template <typename Func, typename... Args>
        ObjectInfo*     FirstThat(Func&& lpfn, Args&&... args);
        template <typename Func, typename... Args>
        ObjectInfo*     UnifiedLoop(Func&& lpfn, Args&&... args);

         forward_iterator  begin();
         forward_iterator  end();
         backward_iterator rbegin();
         backward_iterator rend();

         friend ostream& operator<<(ostream& os, BTree<Trait>& tree) {
             shared_lock<shared_mutex> lock(tree.m_mtx);
             for (auto item: tree)
                 os << item << endl;
             return os;
         }

         friend istream& operator>>(istream& is, BTree<Trait>& tree) {
             keyType key; ObjIDType objID;
             TChar colon;
             while (is >> key >> colon >> objID) {
                 if (colon != ':') { is.setstate(ios::failbit); break; }
                 tree.Insert(key, objID);
             }
             return is;
         }

protected:
        BTNode          m_Root;
       T1             m_Height;  // height of tree
       T1             m_Order;   // order of tree
       TLong            m_NumKeys; // number of keys
       TBool            m_Unique;  // Accept the elements only once ?
       mutable shared_mutex m_mtx;
};

const T1 MaxHeight = 5;
template <typename Trait>
BTree<Trait>::BTree(T1 order, TBool unique)
                                : m_Root(2 * order  + 1, unique),
                                  m_Height(1),
                                  m_Order(order),
                                  m_NumKeys(0),
                                  m_Unique(unique)
{
    m_Root.SetMaxKeysForChilds(order);
}

template <typename Trait>
BTree<Trait>::~BTree()
{
}

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const ObjIDType ObjID)
{
       unique_lock<shared_mutex> lock(m_mtx);
       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       if( error == bt_duplicate )
            return false;
       m_NumKeys++;
       if( error == bt_overflow )
       {
            m_Root.SplitRoot();
            m_Height++;
       }
       return true;
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const ObjIDType ObjID)
{
    unique_lock<shared_mutex> lock(m_mtx);
    bt_ErrorCode error = m_Root.Remove(key, ObjID);
    if( error == bt_duplicate || error == bt_nofound )
        return false;
    m_NumKeys--;

    if( error == bt_rootmerged )
        m_Height--;
    return true;
}

template <typename Trait>
typename BTree<Trait>::ObjIDType BTree<Trait>::Search (const keyType key)
{
    unique_lock<shared_mutex> lock(m_mtx);
    ObjIDType ObjID = -1;
    m_Root.Search(key, ObjID);
    return ObjID;
}


template <typename Trait>
template <typename Func, typename... Args>
void BTree<Trait>::ForEach(Func&& lpfn, Args&&... args)
{
    shared_lock<shared_mutex> lock(m_mtx);
    m_Root.ForEach(lpfn, 0, args...);
}

template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo *
BTree<Trait>::FirstThat(Func&& lpfn, Args&&... args)
{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_Root.FirstThat(lpfn, 0, args...);
}

template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo *
BTree<Trait>::UnifiedLoop(Func&& lpfn, Args&&... args)
{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_Root.UnifiedLoop(lpfn, 0, args...);
}

template <typename Trait>
void BTree<Trait>::Print(ostream &os){
    m_Root.Print(os);
}


// Forward iterator
template <typename Trait>
class btree_forward_iterator : public general_iterator<BTree<Trait>, btree_forward_iterator<Trait>> {
public:
    using Parent  = general_iterator<BTree<Trait>, btree_forward_iterator<Trait>>;
    using MySelf  = btree_forward_iterator<Trait>;
    using BTPage  = BTreePage<Trait>;

private:
    struct Frame { BTPage *page; T1 keyIndex; };

    static constexpr T1 MAX_H = MaxHeight;
    Frame   m_stack[MAX_H];
    T1      m_depth;
    BTPage *m_curPage;
    T1      m_curIndex;

    void doPush(BTPage *p, T1 ki) { m_stack[++m_depth] = {p, ki}; }
    Frame  doPop()                { return m_stack[m_depth--]; }

    void goToFirst() {
        BTree<Trait> *tree = static_cast<BTree<Trait>*>(this->m_pContainer);
        BTPage *page = &tree->m_Root;
        m_depth = -1;

        if (page->GetNumberOfKeys() == 0) {
            m_curPage = nullptr; m_curIndex = -1;
            this->m_pNode = nullptr;
            return;
        }

        while (page->GetSubPage(0)) {
            doPush(page, 0);
            page = page->GetSubPage(0);
        }
        m_curPage = page;
        m_curIndex = 0;
        this->m_pNode = &page->GetKeyRef(0);
    }

    void advance() {
        if (!this->m_pNode) return;

        BTPage *page = m_curPage;
        T1 idx = m_curIndex;

        if (page->GetSubPage(idx + 1)) {
            if (idx + 1 < page->GetNumberOfKeys())
                doPush(page, idx + 1);

            page = page->GetSubPage(idx + 1);
            while (page->GetSubPage(0)) {
                doPush(page, 0);
                page = page->GetSubPage(0);
            }
            m_curPage  = page;
            m_curIndex = 0;
            this->m_pNode = &page->GetKeyRef(0);
        } else if (idx + 1 < page->GetNumberOfKeys()) {
            m_curIndex = idx + 1;
            this->m_pNode = &page->GetKeyRef(m_curIndex);
        } else {
            if (m_depth < 0) {
                this->m_pNode = nullptr;
                return;
            }
            Frame f = doPop();
            m_curPage  = f.page;
            m_curIndex = f.keyIndex;
            this->m_pNode = &f.page->GetKeyRef(f.keyIndex);
        }
    }

public:
    btree_forward_iterator() : Parent(nullptr, nullptr) {
        m_depth = -1; m_curPage = nullptr; m_curIndex = -1;
    }
    explicit btree_forward_iterator(BTree<Trait> *tree) : Parent(tree, nullptr) {
        goToFirst();
    }

    MySelf& operator++() { advance(); return *this; }
};


// Backward iterator
template <typename Trait>
class btree_backward_iterator : public general_iterator<BTree<Trait>, btree_backward_iterator<Trait>> {
public:
    using Parent  = general_iterator<BTree<Trait>, btree_backward_iterator<Trait>>;
    using MySelf  = btree_backward_iterator<Trait>;
    using BTPage  = BTreePage<Trait>;

private:
    struct Frame { BTPage *page; T1 keyIndex; };

    static constexpr T1 MAX_H = MaxHeight;
    Frame   m_stack[MAX_H];
    T1      m_depth;
    BTPage *m_curPage;
    T1      m_curIndex;

    void doPush(BTPage *p, T1 ki) { m_stack[++m_depth] = {p, ki}; }
    Frame  doPop()                { return m_stack[m_depth--]; }

    void goToLast() {
        BTree<Trait> *tree = static_cast<BTree<Trait>*>(this->m_pContainer);
        BTPage *page = &tree->m_Root;
        m_depth = -1;

        if (page->GetNumberOfKeys() == 0) {
            m_curPage = nullptr; m_curIndex = -1;
            this->m_pNode = nullptr;
            return;
        }

        while (true) {
            T1 n = page->GetNumberOfKeys();
            if (page->GetSubPage(n)) {
                doPush(page, n - 1);
                page = page->GetSubPage(n);
            } else {
                break;
            }
        }
        m_curPage  = page;
        m_curIndex = page->GetNumberOfKeys() - 1;
        this->m_pNode = &page->GetKeyRef(m_curIndex);
    }

    void retreat() {
        if (!this->m_pNode) return;

        BTPage *page = m_curPage;
        T1 idx = m_curIndex;

        if (page->GetSubPage(idx)) {
            if (idx - 1 >= 0)
                doPush(page, idx - 1);

            page = page->GetSubPage(idx);
            while (true) {
                T1 n = page->GetNumberOfKeys();
                if (page->GetSubPage(n)) {
                    doPush(page, n - 1);
                    page = page->GetSubPage(n);
                } else {
                    break;
                }
            }
            m_curPage  = page;
            m_curIndex = page->GetNumberOfKeys() - 1;
            this->m_pNode = &page->GetKeyRef(m_curIndex);
        } else if (idx - 1 >= 0) {
            m_curIndex = idx - 1;
            this->m_pNode = &page->GetKeyRef(m_curIndex);
        } else {
            while (m_depth >= 0) {
                Frame f = doPop();
                if (f.keyIndex >= 0) {
                    m_curPage  = f.page;
                    m_curIndex = f.keyIndex;
                    this->m_pNode = &f.page->GetKeyRef(f.keyIndex);
                    return;
                }
            }
            this->m_pNode = nullptr;
        }
    }

public:
    btree_backward_iterator() : Parent(nullptr, nullptr) {
        m_depth = -1; m_curPage = nullptr; m_curIndex = -1;
    }
    explicit btree_backward_iterator(BTree<Trait> *tree) : Parent(tree, nullptr) {
        goToLast();
    }

    MySelf& operator++() { retreat(); return *this; }
};


//  begin 
template <typename Trait>
typename BTree<Trait>::forward_iterator BTree<Trait>::begin() {
    return forward_iterator(this);
}

// end
template <typename Trait>
typename BTree<Trait>::forward_iterator BTree<Trait>::end() {
    return forward_iterator();
}

// rbegin
template <typename Trait>
typename BTree<Trait>::backward_iterator BTree<Trait>::rbegin() {
    return backward_iterator(this);
}

// rend
template <typename Trait>
typename BTree<Trait>::backward_iterator BTree<Trait>::rend() {
    return backward_iterator();
}

#endif