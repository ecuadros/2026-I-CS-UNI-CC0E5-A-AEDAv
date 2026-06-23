#ifndef __TRAITS_H__
#define __TRAITS_H__

#include <functional>

#include "../types.h"

using namespace std;

template <typename _Node, typename _Comp>
struct BaseTrait {
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait
    : BaseTrait<_Node, less<typename _Node::value_type>> {};

template <typename _Node>
struct DescendingTrait
    : BaseTrait<_Node, greater<typename _Node::value_type>> {};

template <typename _Key, typename _Value, typename _Comp = less<_Key>>
struct KVTrait {
    using Key = _Key;
    using Value = _Value;
    using Comp = _Comp;
};

template <typename _Value, Size _Order, typename _Comp = less<_Value>>
struct BTreeTrait {
    using value_type = _Value;
    using Comp = _Comp;
    static constexpr Size Order = _Order;
};

template <typename _Value, typename _Comp = less<_Value>>
using Tree23Trait = BTreeTrait<_Value, 2, _Comp>;

#endif // __TRAITS_H__
