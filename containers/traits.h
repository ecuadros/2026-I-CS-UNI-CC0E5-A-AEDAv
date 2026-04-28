#ifndef __TRAITS_H__
#define __TRAITS_H__

template <typename T, typename _Comp, typename NodeType>
struct BaseTrait{
    using value_type = T;
    using Comp = _Comp;
    using Node = NodeType;
};

#endif // __TRAITS_H__
