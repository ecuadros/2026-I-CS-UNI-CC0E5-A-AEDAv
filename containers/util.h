#ifndef __UTIL_H__
#define __UTIL_H__
#include <ostream>
#include <istream>
#include <limits>
#include "../types.h"
using namespace std;

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

// Reutilizacion operator<<
template <typename Container>
ostream& container_write(ostream& os, Container& c) {
    os << "[";
    auto it = c.begin(), en = c.end();
    bool first = true;
    while (it != en) {
        if (!first) os << ",";
        os << "(" << *it << "," << it.getRef() << ")";
        ++it;
        first = false;
    }
    os << "]";
    return os;
}

// Reutilizacion operator>>
template <typename Container>
istream& container_read(istream& is, Container& c) {
    char ch;
    if (!(is >> ch) || ch != '[') { is.setstate(ios::failbit); return is; }
    typename Container::value_type val;
    Ref ref; char comma, paren;
    // parsea [(val,ref),...]
    while (is >> ch && ch != ']')
        if (ch == '(' && (is >> val >> comma >> ref >> paren) && comma == ',' && paren == ')')
            c.insert(val, ref); // inserta
    is.ignore(numeric_limits<streamsize>::max(), '\n');
    return is;
}

#endif
