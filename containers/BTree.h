// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "BTreePage.h"
#include "traits.h"
#include "../types.h"

#define DEFAULT_BTREE_ORDER 3

template <typename Trait>
class BTree {
public:
       using value_type = typename Trait::value_type;
       using Comp       = typename Trait::Comp;
       using ObjectInfo = BTreeData<value_type>;
       using BTPage     = CBTreePage<Trait>;

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true);
       virtual ~BTree(); 
       bool         Insert(const value_type key, Ref ObjID); 
       bool         Remove(const value_type key, Ref ObjID);
       Ref          Search(const value_type key); // Devuelve Ref en lugar de ObjIDType
       
       size_t       size()   const { return m_NumKeys; }
       size_t       height() const { return m_Height;  }
       size_t       GetOrder() const { return m_Order; }
       void         Print(ostream &os);
       
       template <typename Func, typename... Args>
       void ForEach(Func func, Args&&... args); 

       template <typename Func, typename... Args>
       ObjectInfo* FirstThat(Func func, Args&&... args); 

protected:
       BTPage       m_Root;
       size_t       m_Height; 
       size_t       m_Order;  
       size_t       m_NumKeys;
       bool         m_Unique; 
};

const size_t MaxHeight = 5;
template <typename Trait>
BTree<Trait>::BTree(size_t order, bool unique)
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
bool BTree<Trait>::Insert(const value_type key, Ref ObjID)
{
       bt_ErrorCode error = m_Root.Insert(key, ObjID); // Quienes resuelven los problemas de los hijos son los padres, es decir si un hijo se desborda, el padre lo resuelve
       if (error == bt_duplicate) return false;
       m_NumKeys++;
       if( error == bt_overflow )
       {
               m_Root.SplitRoot();
               m_Height++;
       }
       return true;
}

template <typename Trait>
bool BTree<Trait>::Remove(const value_type key, Ref ObjID)
{
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound ) // posiblemente solo error == bt_nofound, ya que el duplicate no es un error, sino que es un warning
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
Ref BTree<Trait>::Search(const value_type key)
{
       Ref ObjID;
       m_Root.Search(key, ObjID);
       return ObjID;
}


template <typename Trait>
template <typename Func, typename... Args>
void BTree<Trait>::ForEach(Func func, Args&&... args)
{
    m_Root.ForEach(func, 0, std::forward<Args>(args)...);
}

template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo* BTree<Trait>::FirstThat(Func func, Args&&... args)
{
    return m_Root.FirstThat(func, 0, std::forward<Args>(args)...);
}

template <typename Trait>
void BTree<Trait>::Print(ostream &os){
       m_Root.Print(os);
}






#endif