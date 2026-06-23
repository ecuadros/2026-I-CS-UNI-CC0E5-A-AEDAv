#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional>
#include "../types.h"

template <typename T, typename _Comp, typename NodeType>
struct BaseTrait{
    using value_type = T;
    using Comp = _Comp;
    using Node = NodeType;
};

// Trait del BTree: el tipo de la clave + el comparador. ObjID siempre es Ref.
template <typename T>
struct AscendingBTreeTrait{
    using keyType   = T;
    using ObjIDType = Ref;
    using Comp      = std::less<T>;
};
template <typename T>
struct DescendingBTreeTrait{
    using keyType   = T;
    using ObjIDType = Ref;
    using Comp      = std::greater<T>;
};

#endif // __TRAITS_H__
