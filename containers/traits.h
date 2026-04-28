#ifndef __TRAITS_H__
#define __TRAITS_H__

template <typename T, typename _Comp, typename _Node>
struct BaseTrait{//Caracteristicas comunes a Ascending/DescendingCLLTrait
    using value_type = T;//Using es para crear alias de tipos
    using Node = _Node;
    using Comp = _Comp;
};

#endif // __TRAITS_H__