// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <shared_mutex>
#include <mutex>
#include "BTreePage.h"

#define DEFAULT_BTREE_ORDER 3

template <typename TreeType>
class btree_iterator_base
{
public:
       using ObjectInfo = typename TreeType::ObjectInfo;
       using BTNode     = typename TreeType::BTNode;

protected:
       struct Frame { BTNode *page; T1 idx; };
       vector<Frame> m_stack;

public:
       btree_iterator_base() {}

       ObjectInfo& operator*()  const { return m_stack.back().page->m_Keys[m_stack.back().idx]; }
       ObjectInfo* operator->() const { return &m_stack.back().page->m_Keys[m_stack.back().idx]; }

       T1 level() const { return (T1)m_stack.size() - 1; }

       friend bool operator==(const btree_iterator_base &a, const btree_iterator_base &b)
       {
               if( a.m_stack.empty() || b.m_stack.empty() )
                       return a.m_stack.empty() == b.m_stack.empty();
               return a.m_stack.back().page == b.m_stack.back().page &&
                      a.m_stack.back().idx  == b.m_stack.back().idx;
       }
};


template <typename TreeType, bool IsForward>
class btree_iterator : public btree_iterator_base<TreeType>
{
       using Base   = btree_iterator_base<TreeType>;
       using BTNode = typename Base::BTNode;
       using MySelf = btree_iterator<TreeType, IsForward>;

       T1 first_index(BTNode *page) const
       {
               if constexpr (IsForward) return 0;
               else                     return page->NumberOfKeys() - 1;
       }

       bool exhausted(const typename Base::Frame &f) const
       {
               if constexpr (IsForward) return f.idx >= f.page->NumberOfKeys();
               else                     return f.idx < 0;
       }

       void push_extreme(BTNode *page)
       {
               while( page )
               {
                       T1 idx = first_index(page);
                       this->m_stack.push_back({page, idx});
                       page = page->m_SubPages[idx + (IsForward ? 0 : 1)];
               }
       }

       void cleanup()
       {
               while( !this->m_stack.empty() && exhausted(this->m_stack.back()) )
                       this->m_stack.pop_back();
       }

public:
       btree_iterator() {}

       explicit btree_iterator(BTNode *root)
       {
               push_extreme(root);
               cleanup();
       }

       MySelf& operator++()
       {
               auto frame = this->m_stack.back();
               this->m_stack.pop_back();
               BTNode *child = frame.page->m_SubPages[frame.idx + (IsForward ? 1 : 0)];
               this->m_stack.push_back({frame.page, frame.idx + (IsForward ? 1 : -1)});
               if( child )
                       push_extreme(child);
               cleanup();
               return *this;
       }

       MySelf operator++(int) { MySelf tmp = *this; ++(*this); return tmp; }
};

template <typename Trait>
class BTree
// this is the full version of the BTree
{
       using keyType   = typename Trait::keyType;
       using ObjIDType = typename Trait::ObjIDType;
       /*struct ObjectInfo
       {
               keyType first;
               long    second;
               ObjectInfo *&operator->() { return this; }
       };*/

public:
       // Public so btree_iterator_base/btree_iterator<BTree<Trait>, ...> can
       // name them (they are not friends of BTree, only of CBTreePage).
       using BTNode     = CBTreePage<Trait>;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

       using forward_iterator  = btree_iterator<BTree<Trait>, true>;
       using backward_iterator = btree_iterator<BTree<Trait>, false>;

public:
       BTree(T1 order = DEFAULT_BTREE_ORDER, bool unique = true);
       ~BTree();
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       bool            Insert (const keyType key, const Ref ObjID);
       bool            Remove (const keyType key, const Ref ObjID);
       ObjIDType       Search (const keyType key);
       T1              size()  { shared_lock<shared_mutex> lock(m_mtx); return m_NumKeys; }
       T1              height() { shared_lock<shared_mutex> lock(m_mtx); return m_Height; }
       T1              GetOrder() { shared_lock<shared_mutex> lock(m_mtx); return m_Order; }

       void            Print (ostream &os);

       forward_iterator  begin()  { shared_lock<shared_mutex> lock(m_mtx); return begin_impl(); }
       forward_iterator  end()    { shared_lock<shared_mutex> lock(m_mtx); return end_impl(); }
       backward_iterator rbegin() { shared_lock<shared_mutex> lock(m_mtx); return rbegin_impl(); }
       backward_iterator rend()   { shared_lock<shared_mutex> lock(m_mtx); return rend_impl(); }


       template <typename Func, typename... Args>
       void ForEach(Func func, Args&&... args)
       {
               shared_lock<shared_mutex> lock(m_mtx);
               ForEach_impl(func, std::forward<Args>(args)...);
       }

       template <typename Pred, typename... Args>
       ObjectInfo* FirstThat(Pred pred, Args&&... args)
       {
               shared_lock<shared_mutex> lock(m_mtx);
               ObjectInfo *found = nullptr;
               ForEach_impl([&](ObjectInfo &info, auto&&... rest) -> bool {
                       if( pred(info, rest...) )
                       {
                               found = &info;
                               return false;
                       }
                       return true;
               }, std::forward<Args>(args)...);
               return found;
       }

       // Operadores I/O
       friend ostream& operator<<(ostream& os, BTree& tree) {
               shared_lock<shared_mutex> lock(tree.m_mtx);
               os << "[";
               bool first = true;
               for( auto it = tree.begin_impl(); it != tree.end_impl(); ++it )
               {
                       if( !first )
                               os << ",";
                       os << "(" << it->key << "," << it->ObjID << ")";
                       first = false;
               }
               os << "]";
               return os;
       }

       friend istream& operator>>(istream& is, BTree& tree) {
               char ch;
               if( !(is >> ch) || ch != '[' )
               {
                       is.clear(ios_base::failbit);
                       return is;
               }
               keyType key;
               Ref     objID;
               char    comma, parenClose;
               while( is >> ch && ch != ']' )
               {
                       if( ch == '(' )
                       {
                               if( is >> key >> comma >> objID >> parenClose )
                               {
                                       if( comma == ',' && parenClose == ')' )
                                               tree.Insert(key, objID);
                               }
                       }
               }
               return is;
       }

protected:
       forward_iterator  begin_impl()  { return forward_iterator(&m_Root); }
       forward_iterator  end_impl()    { return forward_iterator(); }
       backward_iterator rbegin_impl() { return backward_iterator(&m_Root); }
       backward_iterator rend_impl()   { return backward_iterator(); }

       template <typename Func, typename... Args>
       void ForEach_impl(Func func, Args&&... args)
       {
               for( auto it = begin_impl() ; it != end_impl() ; ++it )
                       if( !func(*it, args...) )
                               break;
       }

       BTNode                  m_Root;
       T1                      m_Height;  // height of tree
       T1                      m_Order;   // order of tree
       T1                      m_NumKeys; // number of keys
       bool                    m_Unique;  // Accept the elements only once ?
       mutable shared_mutex    m_mtx;     // reader-writer lock: shared for reads, unique for writes
};

const T1 MaxHeight = 5;
template <typename Trait>
BTree<Trait>::BTree(T1 order, bool unique)
                               : m_Unique(unique),
                                 m_Order(order),
                                 m_Root(2 * order  + 1, unique),
                                 m_NumKeys(0)
{
       m_Root.SetMaxKeysForChilds(order);
       m_Height = 1;
}

template <typename Trait>
BTree<Trait>::~BTree()
{
}

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const Ref ObjID)
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
bool BTree<Trait>::Remove (const keyType key, const Ref ObjID)
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
       shared_lock<shared_mutex> lock(m_mtx);
       ObjIDType ObjID = -1;
       m_Root.Search(key, ObjID);
       return ObjID;
}

template <typename Trait>
void BTree<Trait>::Print(ostream &os)
{
       shared_lock<shared_mutex> lock(m_mtx);
       for( auto it = begin_impl(); it != end_impl(); ++it )
       {
               for( T1 i = 0; i < it.level(); i++ )
                       os << "\t";
               os << it->key << "->" << it->ObjID << "\n";
       }
}

#endif
