// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <type_traits>
#include "BTreePage.h"

#define DEFAULT_BTREE_ORDER 3

/* Estado comun del iterador: una pila de (pagina, indice de clave).
Una pagina tiene varias claves y varios hijos, asi que un solo puntero no alcanza para saber "cual sigue"; por eso guardamos la pila.*/
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

       friend bool operator==(const btree_iterator_base &a, const btree_iterator_base &b)
       {
               if( a.m_stack.empty() || b.m_stack.empty() )
                       return a.m_stack.empty() == b.m_stack.empty();
               return a.m_stack.back().page == b.m_stack.back().page &&
                      a.m_stack.back().idx  == b.m_stack.back().idx;
       }
};


// Un solo iterador para ambos sentidos: IsForward=true recorre ascendente,
// false descendente. La direccion decide los indices, no se duplica la clase.
template <typename TreeType, bool IsForward>
class btree_iterator : public btree_iterator_base<TreeType>
{
       using Base   = btree_iterator_base<TreeType>;
       using BTNode = typename Base::BTNode;
       using MySelf = btree_iterator<TreeType, IsForward>;

       // primer indice a visitar en una pagina: el izquierdo o el derecho
       T1 first_index(BTNode *page) const
       {
               if constexpr (IsForward) return 0;
               else                     return page->NumberOfKeys() - 1;
       }

       // la pagina del tope ya no tiene mas claves para dar?
       bool exhausted(const typename Base::Frame &f) const
       {
               if constexpr (IsForward) return f.idx >= f.page->NumberOfKeys();
               else                     return f.idx < 0;
       }

       // baja hasta la hoja apilando el hijo extremo de cada pagina
       void push_extreme(BTNode *page)
       {
               while( page )
               {
                       T1 idx = first_index(page);
                       this->m_stack.push_back({page, idx});
                       page = page->m_SubPages[idx + (IsForward ? 0 : 1)];
               }
       }

       // desapila las paginas ya agotadas hasta encontrar una pendiente
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

       // avanza: baja al hijo siguiente si existe, si no sigue en esta pagina
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
       // publicos para que el iterador pueda nombrarlos
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

       forward_iterator  begin()  { return forward_iterator(&m_Root); }
       forward_iterator  end()    { return forward_iterator(); }
       backward_iterator rbegin() { return backward_iterator(&m_Root); }
       backward_iterator rend()   { return backward_iterator(); }

       template <typename Func, typename... Args>
       auto ForEach(Func func, Args&&... args)
       {
               using result_t = invoke_result_t<Func, ObjectInfo&, Args...>;
               shared_lock<shared_mutex> lock(m_mtx);
               for( auto it = begin(); it != end(); ++it )
               {
                       if constexpr (is_void_v<result_t>)
                               func(*it, args...);
                       else if( func(*it, args...) )
                               return &(*it);
               }
               if constexpr (!is_void_v<result_t>)
                       return (ObjectInfo*)nullptr;
       }

       // Operadores I/O
       friend ostream& operator<<(ostream& os, BTree& tree) {
               os << "[";
               bool first = true;
               tree.ForEach([&](auto &info){
                       if( !first )
                               os << ",";
                       os << "(" << info.key << "," << info.ObjID << ")";
                       first = false;
               });
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
       BTNode                  m_Root;
       T1                      m_Height;  // height of tree
       T1                      m_Order;   // order of tree
       T1                      m_NumKeys; // number of keys
       bool                    m_Unique;  // Accept the elements only once ?
       mutable shared_mutex    m_mtx;     // lock lectura/escritura
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

#endif
