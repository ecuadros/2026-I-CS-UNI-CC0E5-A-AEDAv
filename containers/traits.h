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
// (traits)
template <typename _Value, typename _Ref = Ref, typename _Comp = less<_Value>>
struct BTreeTrait {
    using value_type = _Value;
    using ref_type   = _Ref;
    using Comp       = _Comp;
    using compare_type = _Comp;
};
template <typename _Value, typename _Ref = Ref>
using AscendingBTreeTrait = BTreeTrait<_Value, _Ref, less<_Value>>;
template <typename _Value, typename _Ref = Ref>
using DescendingBTreeTrait = BTreeTrait<_Value, _Ref, greater<_Value>>;

#endif // __TRAITS_H__
