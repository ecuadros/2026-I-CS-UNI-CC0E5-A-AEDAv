// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include "BTreePage.h"
#include "traits.h"
#define DEFAULT_BTREE_ORDER 3

using namespace std;
template <typename Trait>
class BTree
{
public:
       using value_type = typename Trait::value_type;
       using ref_type   = typename Trait::ref_type;
       using compare_type = typename Trait::compare_type;
       using keyType   = value_type;
       using ObjIDType = ref_type;
       using Comp      = compare_type;
       using BTNode    = CBTreePage<Trait>;
       using ObjectInfo     = typename BTNode::ObjectInfo;
       using node_type      = ObjectInfo;
       using IndexType      = size_t;
       using StepType       = Ref;
       using LevelType      = typename BTNode::LevelType;
       using Flag           = typename BTNode::Flag;
       using Token          = string::value_type;
       class Iterator
       {
               shared_ptr<vector<ObjectInfo>> m_Items;
               IndexType m_Pos;
               StepType m_Step;

               Flag IsEnd() const
               {
                      return !m_Items || m_Pos >= IndexType(m_Items->size());
               }

       public:
               Iterator() : m_Items(nullptr), m_Pos(0), m_Step(1) {}
               Iterator(shared_ptr<vector<ObjectInfo>> items, IndexType pos, StepType step)
                      : m_Items(items), m_Pos(pos), m_Step(step) {}

              ObjectInfo& operator*() { return (*m_Items)[m_Pos]; }
              ObjectInfo* operator->() { return &(*m_Items)[m_Pos]; }

              Iterator& operator++()
              {
                     if( IsEnd() )
                            return *this;
                     if( m_Step < 0 && m_Pos == 0 )
                     {
                            m_Items.reset();
                            m_Pos = 0;
                            return *this;
                     }
                     m_Pos = IndexType(StepType(m_Pos) + m_Step);
                     if( IsEnd() )
                     {
                            m_Items.reset();
                             m_Pos = 0;
                     }
                     return *this;
              }

              friend Flag operator==(const Iterator &a, const Iterator &b)
              {
                     if( a.IsEnd() && b.IsEnd() )
                            return true;
                     return a.m_Items == b.m_Items && a.m_Pos == b.m_Pos;
              }

              friend Flag operator!=(const Iterator &a, const Iterator &b)
              {
                     return !(a == b);
              }
       };
       using iterator = Iterator;
       using reverse_iterator = Iterator;

public:
       BTree(IndexType order = DEFAULT_BTREE_ORDER, Flag unique = true);
       ~BTree();
       //Open
       //Create
       //Close
       Flag            Insert (const keyType key, const ObjIDType ObjID);
       Flag            Remove (const keyType key, const ObjIDType ObjID);
       ObjIDType       Search (const keyType key);
       IndexType       size()  { shared_lock<shared_mutex> lock(m_Mtx); return m_NumKeys; }
       IndexType       height() { shared_lock<shared_mutex> lock(m_Mtx); return m_Height;  }
       IndexType       GetOrder() { shared_lock<shared_mutex> lock(m_Mtx); return m_Order; }

       void            Print (ostream &os);
       template <typename Func, typename... Args>
       void            ForEach(Func func, Args&&... args);
       template <typename Func, typename... Args>
       ObjectInfo*     FirstThat(Func func, Args&&... args);
       template <typename Func, typename... Args>
       void            ForEachReverse(Func func, Args&&... args);
       template <typename Func, typename... Args>
       ObjectInfo*     FirstThatReverse(Func func, Args&&... args);
       iterator        begin();
       iterator        end();
       reverse_iterator rbegin();
       reverse_iterator rend();
       //typedef               ObjectInfo iterator;

protected:
       BTNode          m_Root;
       IndexType       m_Height;  // height of tree
       IndexType       m_Order;   // order of tree
       IndexType       m_NumKeys; // number of keys
       Flag            m_Unique;  // Accept the elements only once ?
       mutable shared_mutex m_Mtx; // concurrencia
};

const Ref MaxHeight = 5;
template <typename Trait>
BTree<Trait>::BTree(typename BTree<Trait>::IndexType order,
                    typename BTree<Trait>::Flag unique)
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
typename BTree<Trait>::Flag
BTree<Trait>::Insert(const typename BTree<Trait>::keyType key,
                           const typename BTree<Trait>::ObjIDType ObjID)
{
       unique_lock<shared_mutex> lock(m_Mtx);
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
typename BTree<Trait>::Flag
BTree<Trait>::Remove (const typename BTree<Trait>::keyType key,
                            const typename BTree<Trait>::ObjIDType ObjID)
{
       unique_lock<shared_mutex> lock(m_Mtx);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
typename BTree<Trait>::ObjIDType
BTree<Trait>::Search (const typename BTree<Trait>::keyType key)
{
       shared_lock<shared_mutex> lock(m_Mtx);
       ObjIDType ObjID = -1;
       m_Root.Search(key, ObjID);
       return ObjID;
}


template <typename Trait>
template <typename Func, typename... Args>
void BTree<Trait>::ForEach(Func func, Args&&... args)
{
       shared_lock<shared_mutex> lock(m_Mtx);
       m_Root.ForEach(func, 0, args...);
}

template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo *
BTree<Trait>::FirstThat(Func func, Args&&... args)
{
       shared_lock<shared_mutex> lock(m_Mtx);
       return m_Root.FirstThat(func, 0, args...);
}

template <typename Trait>
template <typename Func, typename... Args>
void BTree<Trait>::ForEachReverse(Func func, Args&&... args)
{
       shared_lock<shared_mutex> lock(m_Mtx);
       m_Root.ForEachReverse(func, 0, args...);
}

template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo *
BTree<Trait>::FirstThatReverse(Func func, Args&&... args)
{
       shared_lock<shared_mutex> lock(m_Mtx);
       return m_Root.FirstThatReverse(func, 0, args...);
}

template <typename Trait>
typename BTree<Trait>::iterator BTree<Trait>::begin()
{
       auto items = make_shared<vector<ObjectInfo>>();
       ForEach([items](ObjectInfo &info, LevelType) {
              items->push_back(info);
       });
       if( items->empty() )
              return end();
       return iterator(items, 0, 1);
}

template <typename Trait>
typename BTree<Trait>::iterator BTree<Trait>::end()
{
       return iterator();
}

template <typename Trait>
typename BTree<Trait>::reverse_iterator BTree<Trait>::rbegin()
{
       auto items = make_shared<vector<ObjectInfo>>();
       ForEach([items](ObjectInfo &info, LevelType) {
              items->push_back(info);
       });
       if( items->empty() )
              return rend();
       return reverse_iterator(items, IndexType(items->size()) - 1, -1);
}

template <typename Trait>
typename BTree<Trait>::reverse_iterator BTree<Trait>::rend()
{
       return reverse_iterator();
}

template <typename Trait>
void BTree<Trait>::Print(ostream &os){
       shared_lock<shared_mutex> lock(m_Mtx);
       m_Root.Print(os);
}

// operator <<
template <typename Trait>
ostream& operator<<(ostream& os, BTree<Trait>& tree)
{
       typename BTree<Trait>::Flag first = true;
       os << "[";
       tree.ForEach([&](typename BTree<Trait>::ObjectInfo &info, typename BTree<Trait>::LevelType) {
               if( !first )
                       os << " ";
               os << "(" << info.key << "," << info.ObjID << ")";
               first = false;
       });
       os << "]";
       return os;
}

// operator >>
template <typename Trait>
istream& operator>>(istream& is, BTree<Trait>& tree)
{
       using keyType = typename BTree<Trait>::keyType;
       using ObjIDType = typename BTree<Trait>::ObjIDType;

       typename BTree<Trait>::Token openList;
       if( !(is >> openList) || openList != '[' )
       {
               is.setstate(ios_base::failbit);
               return is;
       }

       is >> ws;
       if( is.peek() == ']' )
       {
               is.get();
               return is;
       }

       while( is )
       {
               typename BTree<Trait>::Token openEntry, comma, closeEntry;
               keyType key;
               ObjIDType objID;

               if( !(is >> openEntry >> key >> comma >> objID >> closeEntry) ||
                   openEntry != '(' || comma != ',' || closeEntry != ')' )
               {
                       is.setstate(ios_base::failbit);
                       return is;
               }

               tree.Insert(key, objID);

               is >> ws;
               if( is.peek() == ']' )
               {
                       is.get();
                       return is;
               }
       }
       return is;
}






#endif
