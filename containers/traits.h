#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional> // para less y greater
#include "../types.h"

template <typename _Node, typename _Comp>
struct BaseTrait{
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, std::less<typename _Node::value_type>>{
};
template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, std::greater<typename _Node::value_type>>{
};

// P1 Tarea Traits: BTreeTrait encapsula value_type, Order y Comp en un solo template param
template <typename _Value, Size _Order, typename _Comp = std::less<_Value>>
struct BTreeTrait {
    using value_type            = _Value;
    using Comp                  = _Comp;
    static constexpr Size Order = _Order;
};

// Aliases: arbol 2-3 (order 2) y arbol 3-4 (order 3)
template <typename _Value, typename _Comp = std::less<_Value>>
using Tree23Trait = BTreeTrait<_Value, 2, _Comp>;

template <typename _Value, typename _Comp = std::less<_Value>>
using Tree34Trait = BTreeTrait<_Value, 3, _Comp>;

#endif // __TRAITS_H__