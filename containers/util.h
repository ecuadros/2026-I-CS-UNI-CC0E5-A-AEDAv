#ifndef __UTIL_H__
#define __UTIL_H__
#include <ostream>
#include <functional>    // invoke
#include <type_traits>   // invoke_result_t, is_void_v
#include <utility>       // forward
using namespace std;

// invoca cualquier callable y maneja uniformemente retorno void vs valor
template<typename Callable, typename... Args>
decltype(auto) call(Callable func, Args&&... args){
    if constexpr(is_void_v<invoke_result_t<Callable, Args...>>)
        invoke(forward<Callable>(func), forward<Args>(args)...);        // void: solo invoca
    else
        return invoke(forward<Callable>(func), forward<Args>(args)...); // valor: lo devuelve
}

template <typename Container>
void Print(Container& c, ostream &os){
    os << c << endl;
}

template <typename T>
void PrintX(T& elem, ostream &os, string sep){
    os << elem << sep;
}

template <typename Container, typename Func>
void ForEach(Container& c, Func func){
    for(auto it = c.begin(); it != c.end(); ++it)
        func(*it);
}

template <typename Iterator, typename Func, typename... Args>
void ForEach(Iterator begin, Iterator end, Func func, Args&&... args){
    for(auto it = begin; it != end; ++it)
        func(*it, forward<Args>(args)...);
}

template <typename Container, typename Func, typename... Args>
void ForEach(Container& container, Func func, Args&&... args){
    ForEach(container.begin(), container.end(),
            func, forward<Args>(args)...);
}
#endif