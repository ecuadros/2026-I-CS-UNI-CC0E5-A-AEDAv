#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional>

template <typename _Node>
struct NodeTrait{
    using Node       = _Node;
    using value_type = typename _Node::value_type;
};

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

#endif // __TRAITS_H__
