#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional> // para less y greater
#include "../types.h"
using namespace std;

template <typename _Node, typename _Comp>
struct BaseTrait{
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, less<typename _Node::value_type>>{
};
template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, greater<typename _Node::value_type>>{
};

template <typename keyType, typename ObjIDType>
struct tagObjectInfo;

template <typename _keyType, typename _ObjIDType = Ref,
                 typename _Comp  = less<_keyType>,
                 typename _Entry = tagObjectInfo<_keyType, _ObjIDType>>
struct BTreeTrait
{
    using keyType   = _keyType;
    using ObjIDType = _ObjIDType;
    using Comp      = _Comp;
    using Entry     = _Entry;
};

#endif // __TRAITS_H__