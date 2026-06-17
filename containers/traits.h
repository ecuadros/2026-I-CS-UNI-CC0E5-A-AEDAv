#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional> // para less y greater

template <typename _Node, typename _Comp>
struct BaseTrait{
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;// comparador para mantener el orden
};

template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, less<typename _Node::value_type>>{
};
template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, greater<typename _Node::value_type>>{
};

template <typename _Key, typename _Value>
struct KeyValueTrait {
    using Value = _Value;
    using Key   = _Key;
}; 

#endif // __TRAITS_H__