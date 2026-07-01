// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include "BTreePage.h"
#include "traits.h"
#include "../types.h"
#include "BTreeIterator.h"

#define DEFAULT_BTREE_ORDER 3

template <typename Trait>
class BTree {
public:
       using MySelf     = BTree<Trait>;
       using value_type = typename Trait::value_type;
       using Comp       = typename Trait::Comp;
       using ObjectInfo = BTreeData<value_type>;
       using BTPage     = CBTreePage<Trait>;
       using iterator         = BTreeForwardIterator<MySelf>;
       using reverse_iterator = BTreeBackwardIterator<MySelf>;

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true);
       virtual ~BTree(); 
       bool         Insert(const value_type key, Ref ObjID); 
       bool         Remove(const value_type key, Ref ObjID);
       Ref          Search(const value_type key); // Devuelve Ref en lugar de ObjIDType
       
       size_t       size()   const { std::shared_lock<std::shared_mutex> lock(m_mtx); return m_NumKeys; }
       size_t       height() const { std::shared_lock<std::shared_mutex> lock(m_mtx); return m_Height;  }
       size_t       GetOrder() const { std::shared_lock<std::shared_mutex> lock(m_mtx); return m_Order; }
       
       iterator begin() { return iterator(&m_Root, false); }
       iterator end()   { return iterator(&m_Root, true);  }
       reverse_iterator rbegin() { return reverse_iterator(&m_Root, false); }
       reverse_iterator rend()   { return reverse_iterator(&m_Root, true);  }

       template <typename Func, typename... Args>
       void ForEach(Func func, Args&&... args); 

       template <typename Func, typename... Args>
       ObjectInfo* FirstThat(Func func, Args&&... args); 

       friend std::ostream& operator<<(std::ostream& os, MySelf& tree)
       {
           std::shared_lock<std::shared_mutex> lock(tree.m_mtx);
           os << "[";
           bool first = true;
           for (auto it = tree.begin(); it != tree.end(); ++it) {
               if (!first) os << ", ";
               os << *it; 
               first = false;
           }
           os << "]";
           return os;
       }

       friend std::istream& operator>>(std::istream& is, MySelf& bt)
       {
           Character ch;
           if (!(is >> ch) || ch != '[') { 
              is.setstate(std::ios_base::failbit); 
              return is;
           }

           if ((is >> std::ws).peek() == ']') { 
              is >> ch; 
              return is; 
           }
           
           typename MySelf::ObjectInfo temp_data;
           while (is >> temp_data) {
              bt.Insert(temp_data.key, temp_data.ref);
              is >> ch;
              if (ch == ']') break;
              if (ch != ',') 
              { 
                     is.setstate(std::ios_base::failbit); 
                     break; 
              }
           }
           return is;
       }

protected:
       BTPage       m_Root;
       size_t       m_Height; 
       size_t       m_Order;  
       size_t       m_NumKeys;
       bool         m_Unique; 
       mutable shared_mutex m_mtx;

       template <typename Func, typename... Args>
       auto call(Func func, Args&&... args);
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
       std::unique_lock<std::shared_mutex> lock(m_mtx);
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
       std::unique_lock<std::shared_mutex> lock(m_mtx);
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
       std::shared_lock<std::shared_mutex> lock(m_mtx);
       Ref ObjID;
       m_Root.Search(key, ObjID);
       return ObjID;
}

template <typename Trait>
template <typename Func, typename... Args>
auto BTree<Trait>::call(Func func, Args&&... args)
{
       using RetType = std::invoke_result_t<Func, ObjectInfo&, size_t, Args...>;
       constexpr bool is_void = std::is_void_v<RetType>;
       std::shared_lock<std::shared_mutex> lock(m_mtx);

       for (auto it = begin(); it != end(); ++it) 
       {
           if constexpr (is_void) {
               std::invoke(func, *it, 0, std::forward<Args>(args)...);
           } else {
               if (std::invoke(func, *it, 0, std::forward<Args>(args)...)) {
                   return &(*it);
               }
           }
       }

       if constexpr (!is_void) {
           return static_cast<ObjectInfo*>(nullptr);
       }
}

template <typename Trait>
template <typename Func, typename... Args>
void BTree<Trait>::ForEach(Func func, Args&&... args)
{
    call(func, std::forward<Args>(args)...);
}

template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo* BTree<Trait>::FirstThat(Func func, Args&&... args)
{
    return call(func, std::forward<Args>(args)...);
}

// Elimnando el Print Helper






#endif