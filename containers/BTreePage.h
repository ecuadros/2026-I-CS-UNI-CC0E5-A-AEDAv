
//CBTreePage.h

/*************************
#ifndef BTPage_H
#define BTPage_H
***************************/
#ifndef CBTreePage_H
#define CBTreePage_H
#include <vector>
#include <iostream>
#include <assert.h>
#include <functional>
#include <utility>
#include "traits.h"
#include "../types.h"

template <typename Trait>
class BTree;

template <typename TreeType>
class btree_iterator_base;

template <typename TreeType, bool IsForward>
class btree_iterator;

using namespace std;
enum bt_ErrorCode {bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged};

/*template <typename keyType>
bool operator>=(const _ObjectInfo<keyType>& object1, const _ObjectInfo<keyType>& object2)
{ return object1.key >= object2.key;    }

template <typename keyType>
bool operator<=(const _ObjectInfo<keyType>& object1, const _ObjectInfo<keyType>& object2)
{ return object1.key <= object2.key;    }*/

template <typename keyType, typename ObjIDType>
struct tagObjectInfo
{
       keyType                 key;
       ObjIDType               ObjID;
       T1                      UseCounter;
       tagObjectInfo(const keyType     &_key, ObjIDType _ObjID)
               : key(_key), ObjID(_ObjID), UseCounter(0) {}
       tagObjectInfo()                          {}
       operator keyType                         ()     { return key; }
       T1                      GetUseCounter() { return UseCounter;    }
};

template <typename Trait>
class CBTreePage
// this is the in-memory version of the CBTreePage
{
       using keyType   = typename Trait::keyType;
       using ObjIDType = typename Trait::ObjIDType;
       using Comp      = typename Trait::Comp;

       friend class BTree<Trait>;

       // el iterador (en BTree.h) recorre m_Keys/m_SubPages directo
       template <typename TreeType>
       friend class btree_iterator_base;

       template <typename TreeType, bool IsForward>
       friend class btree_iterator;

       typedef CBTreePage<Trait>    BTPage;         // useful shorthand
       typedef typename Trait::Entry ObjectInfo;

 public:
       CBTreePage(T1 maxKeys, bool unique = true);
       virtual ~CBTreePage();

       bt_ErrorCode    Insert (const keyType &key, const ObjIDType ObjID);
       bt_ErrorCode    Remove (const keyType &key, const ObjIDType ObjID);
       bool            Search (const keyType &key, Ref &ObjID);

protected:
       T1   m_MinKeys; // minimum number of keys in a node
       T1   m_MaxKeys, // maximum number of keys in a node
                m_MaxKeysForChilds; // just to distinguish the root
       bool m_Unique;
       bool m_isRoot;
       Comp m_comp;
       vector<ObjectInfo> m_Keys;
       vector<BTPage *>   m_SubPages;
       T1   m_KeyCount;
       void  Create();
       void  Reset ();
       void  Destroy () {   Reset(); delete this;}
       void  clear ();

       bool  Redistribute1   (T1 &pos);
       bool  Redistribute2   (T1 pos);
       void  RedistributeR2L (T1 pos);
       void  RedistributeL2R (T1 pos);

       bool    TreatUnderflow  (T1 &pos)
       {       return Redistribute1(pos) || Redistribute2(pos);}

       bt_ErrorCode    Merge  (T1 pos);
       bt_ErrorCode    MergeRoot ();
       void  SplitChild (T1 pos);

       ObjectInfo &GetFirstObjectInfo();

       bool Overflow()  { return m_KeyCount > m_MaxKeys; }
       bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
       bool IsFull()    { return m_KeyCount >= m_MaxKeys; }
       T1   MinNumberOfKeys()  { return 2*m_MaxKeys/3.0; }
       T1   GetFreeCells()  { return m_MaxKeys - m_KeyCount; }
       T1&  NumberOfKeys()  { return m_KeyCount; }
       T1   GetNumberOfKeys()  { return m_KeyCount; }
       bool IsRoot()  { return m_MaxKeysForChilds != m_MaxKeys; }
       void SetMaxKeysForChilds(T1 orderforchilds)
       {
               m_MaxKeysForChilds = orderforchilds;
       }

       T1 GetFreeCellsOnLeft(T1 pos);
       T1 GetFreeCellsOnRight(T1 pos);

private:
       bool SplitRoot();
       void SplitPageInto3(vector<ObjectInfo>   & tmpKeys,
                                               vector<BTPage *>  & SubPages,
                                               BTPage           *& pChild1,
                                               BTPage           *& pChild2,
                                               BTPage           *& pChild3,
                                               ObjectInfo        & oi1,
                                               ObjectInfo        & oi2);
       void MovePage(BTPage *  pChildPage,vector<ObjectInfo> & tmpKeys,vector<BTPage *> & tmpSubPages);
};

// Si no lo encuentra, deberia decirme:
// cual es la posicion donde deberia estar
template <typename Container, typename ObjType, typename Comp = less<ObjType>>
T1 binary_search(Container& container, T1 first, T1 last, ObjType &object, Comp comp = Comp())
{
       if( first >= last )
               return first;
       while( first < last )
       {
               T1 mid = (first+last)/2;
               ObjType midKey = (ObjType)container[mid];
               if( !comp(object, midKey) && !comp(midKey, object) ) // object == midKey
                       return mid;
               if( comp(midKey, object) ) // object > midKey
                       first = mid+1;
               else
                       last  = mid;
       }
       if( !comp((ObjType)container[first], object) ) // object <= container[first]
               return first;
       return last;
}

template <typename Container, typename ObjType>
void insert_at(Container& container, const ObjType &object, T1 pos)
{
       T1 size = container.size();
       for(T1 i = size-2 ; i >= pos ; i--)
               container[i+1] = container[i];
       container[pos] =  object;

}

template <typename Container>
void remove(Container& container, T1 pos)
{
       T1 size = container.size();
       for(T1 i = pos+1 ; i < size ; i++)
               container[i-1] = container[i];
}

template <typename Trait>
CBTreePage<Trait>:: CBTreePage(T1 maxKeys, bool unique)
                                       : m_MaxKeys(maxKeys), m_Unique(unique), m_KeyCount(0)
{
       Create();
       SetMaxKeysForChilds(m_MaxKeys);
}

template <typename Trait>
CBTreePage<Trait>::~CBTreePage()
{
       Reset();
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Insert(const keyType& key, const ObjIDType ObjID)
{
       T1 pos = binary_search(m_Keys, 0, m_KeyCount, key, m_comp);
       bt_ErrorCode error = bt_ok;

       if( pos < m_KeyCount && !m_comp((keyType)m_Keys[pos], key) && !m_comp(key, (keyType)m_Keys[pos]) && m_Unique)
               return bt_duplicate; // this key is duplicate

       if( !m_SubPages[pos] ) // this is a leave
       {
               ::insert_at(m_Keys, ObjectInfo(key, ObjID), pos);
               NumberOfKeys()++;
               if( Overflow() )
                       return bt_overflow;
               return bt_ok;
       }
       else
       {
               // recursive insertion
               error = m_SubPages[pos]->Insert(key, ObjID);
               if( error == bt_overflow )
               {
                       if( !Redistribute1(pos) )
                               SplitChild(pos);
                       if( Overflow() )          // Propagate overflow
                               return bt_overflow;
                       return bt_ok;
               }
       }

       // Nunca va a entrar a este If porque esta situacion
       // debe haber sido tratada en el if anterior
       if( Overflow() ) // node overflow
               return bt_overflow;
       return bt_ok;
}

template <typename Trait>
bool CBTreePage<Trait>::Redistribute1(T1 &pos)
{
       if( m_SubPages[pos]->Underflow() )
       {       // nkol = Number of keys on left brother, nkor = Number of keys on right brother
               T1 nkol = 0,
                   nkor = 0;
               // is this the first element or there are more elements on right brother
               if( pos > 0 )
                       nkol = m_SubPages[pos-1]->NumberOfKeys();
               if( pos < NumberOfKeys() )
                       nkor = m_SubPages[pos+1]->NumberOfKeys();

               if( nkol > nkor )
                       if( m_SubPages[pos-1]->NumberOfKeys() > m_SubPages[pos-1]->MinNumberOfKeys() )
                               RedistributeL2R(pos-1); // bring elements from left brother
                       else
                               if( pos == NumberOfKeys() )
                                       return (--pos, false);
                               else
                                       return false;
               else //nkol < nkor )
                       if( m_SubPages[pos+1]->NumberOfKeys() > m_SubPages[pos+1]->MinNumberOfKeys() )
                               RedistributeR2L(pos+1); // bring elements from right brother
                       else
                               if( pos == 0 )
                                       return (++pos, false);
                               else
                                       return false;
       }
       else // it is due to overflow
       {
               T1 fcol = GetFreeCellsOnLeft(pos),   // Free Cells On Left
                   fcor = GetFreeCellsOnRight(pos);  // Free Cells On Right

               if( !fcol && !fcor && m_SubPages[pos]->IsFull() )
                       return false;
               if( fcol > fcor ) // There is more space on left
                       RedistributeR2L(pos);
               else
                       RedistributeL2R(pos);

       }
       return true;
}

// Redistribute2 function
// it considers two brothers m_SubPages[pos-1] && m_SubPages[pos+1]
// if it fails the only way is merge !
template <typename Trait>
bool CBTreePage<Trait>::Redistribute2(T1 pos)
{
       assert( pos > 0 && pos < NumberOfKeys()  );
       assert( m_SubPages[pos-1] != 0 && m_SubPages[pos] != 0 && m_SubPages[pos+1] != 0 );
       assert( m_SubPages[pos-1]->Underflow() ||
                       m_SubPages[ pos ]->Underflow() ||
                       m_SubPages[pos+1]->Underflow() );

       if( m_SubPages[pos-1]->Underflow() )
       {       // Rotate R2L
               RedistributeR2L(pos+1);
               RedistributeR2L(pos);
               if( m_SubPages[pos-1]->Underflow() )
                       return false;
       }
       else if( m_SubPages[pos+1]->Underflow() )
       {       // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeL2R(pos);
               if( m_SubPages[pos+1]->Underflow() )
                       return false;
       }
       else // The problem is exactly at pos !
       {
               // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeR2L(pos+1);
               if( m_SubPages[pos]->Underflow() )
                       return false;
       }
       return true;
}

template <typename Trait>
void CBTreePage<Trait>::RedistributeR2L(T1 pos)
{
       BTPage  *pSource = m_SubPages[ pos ],
                       *pTarget = m_SubPages[pos-1];

       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
             pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-left page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos-1], pTarget->NumberOfKeys()++);
               // Move the pointer leftest pointer to the rightest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[0], pTarget->NumberOfKeys());

               // Move the leftest element to the root
               m_Keys[pos-1] = pSource->m_Keys[0];

               // Remove the leftest element from rigth page
               ::remove(pSource->m_Keys    , 0);
               ::remove(pSource->m_SubPages, 0);
               pSource->NumberOfKeys()--;
       }
}

template <typename Trait>
void CBTreePage<Trait>::RedistributeL2R(T1 pos)
{
       BTPage  *pSource = m_SubPages[pos],
                       *pTarget = m_SubPages[pos+1];
       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
                 pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-RIGHT page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos], 0);
               // Move the pointer rightest pointer to the leftest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[pSource->NumberOfKeys()], 0);
               pTarget->NumberOfKeys()++;

               // Move the rightest element to the root
               m_Keys[pos] = pSource->m_Keys[pSource->NumberOfKeys()-1];

               // Remove the leftest element from rigth page
               // it is not necessary erase because m_KeyCount controls
               pSource->NumberOfKeys()--;
       }
}

template <typename Trait>
void CBTreePage<Trait>::SplitChild(T1 pos)
{
       // FIRST: deciding the second page to split
       BTPage  *pChild1 = 0, *pChild2 = 0;
       if( pos > 0 )                                   // is left page full ?
               if( m_SubPages[pos-1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos-1];
                       pChild2 = m_SubPages[pos--];
               }
       if( pos < GetNumberOfKeys() )   // is right page full ?
               if( m_SubPages[pos+1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos];
                       pChild2 = m_SubPages[pos+1];
               }

       T1 nKeys = pChild1->GetNumberOfKeys() + pChild2->GetNumberOfKeys() + 1;

       // SECOND: copy both pages to a temporal one
       // Create two tmp vector
       vector<ObjectInfo> tmpKeys;
       //tmpKeys.resize(nKeys);
       vector<BTPage *>   tmpSubPages;
       //tmpKeys.resize(nKeys+1);

       // Prepara el vectpor unificado de las 2 paginas a ser divididas en 3
       // copy from left child
       MovePage(pChild1, tmpKeys, tmpSubPages);
       // copy a key from parent
       tmpKeys    .push_back(m_Keys[pos]);

       // copy from right child
       MovePage(pChild2, tmpKeys, tmpSubPages);

       BTPage *pChild3 = 0;
       ObjectInfo oi1, oi2;
       SplitPageInto3(tmpKeys, tmpSubPages, pChild1, pChild2, pChild3, oi1, oi2);

       // copy the first element to the root
       m_Keys    [pos] = oi1;
       m_SubPages[pos] = pChild1;

       // copy the second element to the root
       ::insert_at(m_Keys, oi2, pos+1);
       ::insert_at(m_SubPages, pChild2, pos+1);
       NumberOfKeys()++;

       m_SubPages[pos+2] = pChild3;
}

template <typename Trait>
void CBTreePage<Trait>::SplitPageInto3(vector<ObjectInfo>& tmpKeys,
                                                vector<BTPage *>  & tmpSubPages,
                                                BTPage*                   &     pChild1,
                                                BTPage*                   &     pChild2,
                                                BTPage*                   &     pChild3,
                                                ObjectInfo                & oi1,
                                                ObjectInfo                & oi2)
{
       assert(tmpKeys.size() >= 8);
       assert(tmpSubPages.size() >= 9);
       if( !pChild1 )
               pChild1 = new BTPage(m_MaxKeysForChilds, m_Unique);

       // Split tmpKeys page into 3 pages
       // copy 1/3 elements to the first child
       pChild1->clear();
       T1 nKeys = (tmpKeys.size()-2)/3;
       T1 i = 0;
       for( ; i < nKeys; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       // first element to go up !
       oi1 = tmpKeys[i++];

       if( !pChild2 )
               pChild2 = new BTPage(m_MaxKeysForChilds, m_Unique);
       pChild2->clear();
       // copy 1/3 to the second child
       nKeys += (tmpKeys.size()-2)/3 + 1;
       T1 j = 0;
       for(; i < nKeys; i++, j++ )
       {
               pChild2->m_Keys    [j] = tmpKeys    [i];
               pChild2->m_SubPages[j] = tmpSubPages[i];
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[j] = tmpSubPages[i];

       // copy the second element to the root
       oi2 = tmpKeys[i++];

       // copy 1/3 to the third child
       if( !pChild3 )
               pChild3 = new BTPage(m_MaxKeysForChilds, m_Unique);
       pChild3->clear();
       nKeys = tmpKeys.size();
       for(j = 0; i < nKeys; i++, j++)
       {
               pChild3->m_Keys    [j] = tmpKeys    [i];
               pChild3->m_SubPages[j] = tmpSubPages[i];
               pChild3->NumberOfKeys()++;
       }
       pChild3->m_SubPages[j] = tmpSubPages[i];
}

template <typename Trait>
bool CBTreePage<Trait>::SplitRoot()
{
       BTPage  *pChild1 = 0, *pChild2 = 0, *pChild3 = 0;
       ObjectInfo oi1, oi2;
       SplitPageInto3( m_Keys,m_SubPages,pChild1, pChild2, pChild3, oi1, oi2);
       clear();

       // copy the first element to the root
       m_Keys    [0] = oi1;
       m_SubPages[0] = pChild1;
       NumberOfKeys()++;

       // copy the second element to the root
       m_Keys    [1] = oi2;
       m_SubPages[1] = pChild2;
       NumberOfKeys()++;

       m_SubPages[2] = pChild3;
       return true;
}

template <typename Trait>
bool CBTreePage<Trait>::Search(const keyType &key, Ref &ObjID)
{
       T1 pos = binary_search(m_Keys, 0, m_KeyCount, key, m_comp);
       if( pos >= m_KeyCount ){
               if( m_SubPages[pos] )
                       return m_SubPages[pos]->Search(key, ObjID);
               else
                       return false;
       }
       if( !m_comp(key, m_Keys[pos].key) && !m_comp(m_Keys[pos].key, key) ) // key == m_Keys[pos].key
       {
               ObjID = m_Keys[pos].ObjID;
               m_Keys[pos].UseCounter++;
               return true;
       }
       if( m_comp(key, m_Keys[pos].key) ) // key < m_Keys[pos].key
               if( m_SubPages[pos] )
                       return m_SubPages[pos]->Search(key, ObjID);
       return false;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Remove(const keyType &key, const ObjIDType ObjID)
{
       bt_ErrorCode error = bt_ok;
       T1 pos = binary_search(m_Keys, 0, m_KeyCount, key, m_comp);
       if( pos < NumberOfKeys() && !m_comp(key, m_Keys[pos].key) && !m_comp(m_Keys[pos].key, key) /*&& m_Keys[pos].m_ObjID == ObjID*/) // We found it !
       {
               // This is a leave: First
               if( !m_SubPages[pos+1] )  // This is a leave ? FIRST CASE !
               {
                       ::remove(m_Keys, pos);
                       NumberOfKeys()--;
                       if( Underflow() )
                               return bt_underflow;
                       return bt_ok;
               }

               // We FOUND IT BUT it is NOT a leave ? SECOND CASE !
               {
                       // Get the first element from right branch
                       ObjectInfo &rFirstFromRight = m_SubPages[pos+1]->GetFirstObjectInfo();
                       // change with a leave
                       swap(m_Keys[pos], rFirstFromRight);
                       // Remove it from this leave

                       //Print(cout);
                       error = m_SubPages[++pos]->Remove(key, ObjID);
               }
       }
       else if( pos == NumberOfKeys() ) // it is not here, go by the last branch
               error = m_SubPages[pos]->Remove(key, ObjID);
       else if( !m_comp(m_Keys[pos].key, key) ){ // key <= m_Keys[pos].key; = is because identical keys are inserted on left (see Insert)
               if( m_SubPages[pos] )
                       error = m_SubPages[pos]->Remove(key, ObjID);
               else
                       return bt_nofound;
       }
       if( error == bt_underflow ){
               // THIRD CASE: After removing the element we have an underflow
               //Print(cout);
               if( TreatUnderflow(pos) )
                       return bt_ok;
               // FOURTH CASE: it was not possible to redistribute -> Merge
               if( IsRoot() && NumberOfKeys() == 2 )
                       return MergeRoot();
               return Merge(pos);
       }
       if( error == bt_nofound )
               return bt_nofound;
       return bt_ok;
}


template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Merge(T1 pos)
{
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                m_SubPages[ pos ]->NumberOfKeys() +
                m_SubPages[pos+1]->NumberOfKeys() ==
                3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       // FIRST: Put all the elements into a vector
       vector<ObjectInfo> tmpKeys;
       //tmpKeys.resize(nKeys);
       vector<BTPage *>   tmpSubPages;

       BTPage  *pChild1 = m_SubPages[pos-1],
                       *pChild2 = m_SubPages[ pos ],
                       *pChild3 = m_SubPages[pos+1];
       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);
       pChild3->Destroy();;

       // Move 1/2 elements to pChild1
       T1 nKeys = pChild1->GetFreeCells();
       T1 i = 0;
       for( ; i < nKeys ; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       m_Keys    [pos-1] = tmpKeys[i];
       m_SubPages[pos-1] = pChild1;

       ::remove(m_Keys    , pos);
       ::remove(m_SubPages, pos);
       NumberOfKeys()--;

       nKeys = pChild2->GetFreeCells();
       T1 j = ++i;
       for(i = 0 ; i < nKeys ; i++, j++ )
       {
               pChild2->m_Keys    [i] = tmpKeys    [j];
               pChild2->m_SubPages[i] = tmpSubPages[j];
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[i] = tmpSubPages[j];
       m_SubPages[ pos ]          = pChild2;

       if( Underflow() )
               return bt_underflow;
       return bt_ok;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::MergeRoot()
{
       T1 pos = 1;
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                       m_SubPages[ pos ]->NumberOfKeys() +
                       m_SubPages[pos+1]->NumberOfKeys() ==
                       3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       BTPage  *pChild1 = m_SubPages[pos-1], *pChild2 = m_SubPages[ pos ], *pChild3 = m_SubPages[pos+1];
       T1 nKeys = pChild1->NumberOfKeys() + pChild2->NumberOfKeys() + pChild3->NumberOfKeys() + 2;

       // FIRST: Put all the elements into a vector
       vector<ObjectInfo> tmpKeys;
       //tmpKeys.resize(nKeys);
       vector<BTPage *>   tmpSubPages;

       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);

       clear();
       T1 i = 0;
       for( ; i < nKeys ; i++ ){
               m_Keys    [i] = tmpKeys    [i];
               m_SubPages[i] = tmpSubPages[i];
               NumberOfKeys()++;
       }
       m_SubPages[i] = tmpSubPages[i];

       //Print(cout);
       pChild1->Destroy();
       pChild2->Destroy();
       pChild3->Destroy();

       return bt_rootmerged;
}

template <typename Trait>
typename CBTreePage<Trait>::ObjectInfo &
CBTreePage<Trait>::GetFirstObjectInfo()
{
       if( m_SubPages[0] )
               return m_SubPages[0]->GetFirstObjectInfo();
       return m_Keys[0];
}

template <typename Trait>
void CBTreePage<Trait>::Create()
{
       Reset();
       m_Keys.resize(m_MaxKeys+1);
       m_SubPages.resize(m_MaxKeys+2, NULL);
       m_KeyCount = 0;
       m_MinKeys  = 2 * m_MaxKeys/3;
}

template <typename Trait>
void CBTreePage<Trait>::Reset()
{
       for( T1 i = 0 ; i < m_KeyCount ; i++ )
               delete m_SubPages[i];
       clear();
}

template <typename Trait>
void CBTreePage<Trait>::clear()
{
       //m_Keys.clear();
       //m_SubPages.clear();
       m_KeyCount = 0;
}

template <typename Trait>
CBTreePage<Trait> * CreateBTreeNode (T1 maxKeys, T1 unique)
{
       return new CBTreePage<Trait> (maxKeys, unique);
}

template <typename Trait>
void CBTreePage<Trait>::MovePage(BTPage *pChildPage, vector<ObjectInfo> &tmpKeys,vector<BTPage *> &tmpSubPages)
{
       T1 nKeys = pChildPage->GetNumberOfKeys();
       T1 i = 0;
       for( ; i < nKeys; i++ )
       {
               tmpKeys    .push_back(pChildPage->m_Keys[i]);
               tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       }
       tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       pChildPage->clear();
}

template <typename Trait>
T1 CBTreePage<Trait>::GetFreeCellsOnLeft(T1 pos)
{
       if( pos > 0 )                                   // there is some page on left ?
               return m_SubPages[pos-1]->GetFreeCells();
       return 0;
}

template <typename Trait>
T1 CBTreePage<Trait>::GetFreeCellsOnRight(T1 pos)
{
       if( pos < GetNumberOfKeys() )   // there is some page on right ?
               return m_SubPages[pos+1]->GetFreeCells();
       return 0;
}

#endif
