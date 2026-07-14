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

template <typename _value_type, typename _Comp, typename _objIdType>
struct BTreeTrait{
    using value_type = _value_type;
    using Comp       = _Comp;
    using objIdType  = _objIdType;

};

template <typename _value_type, typename _Comp = std::greater<_value_type>, typename _objIdType = T1>
struct BTreeDescendingTrait: public BTreeTrait<_value_type, _Comp, _objIdType>{
};

template <typename _value_type, typename _Comp = std::less<_value_type>, typename _objIdType = T1>
struct BTreeAscendingTrait: public BTreeTrait<_value_type, _Comp, _objIdType>{
};


// Traits de grafo por defecto
struct DefaultNodeTraits{
    using id_type    = T1;
    using value_type = Type;
};

struct DefaultEdgeTraits{
    using id_type      = T1;
    using node_id_type = T1;
    using weight_type  = T1;
};

// Traits de grafo especificos
struct MyNodeTrait { using id_type = T1; using value_type = T1; };
struct MyEdgeTrait { using id_type = T1; using node_id_type = T1; using weight_type = T1; };

#endif // __TRAITS_H__