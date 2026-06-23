#ifndef DEMO_UTILS_H
#define DEMO_UTILS_H

#include <iostream>
#include <sstream>
#include <string_view>
#include <utility>

using namespace std;

inline void printTitle(string_view title, string_view border = "--") {
    cout << '\n' << border << ' ' << title << ' ' << border << '\n';
}

template <typename Container>
void testIO(const Container& c) {
    printTitle("operator<< / operator>>");
    stringstream ss;
    ss << c;

    Container copy;
    ss >> copy;

    cout << "  serializado: " << ss.str() << '\n';
    cout << "  restaurado:  " << copy << '\n';
}

template <typename Container, typename Func>
void testCopyMove(const Container& c, Func func) {
    printTitle("constructores copy / move");

    Container copy(c);
    func(copy);
    cout << "  original size=" << c.size()
         << "  copy modificada size=" << copy.size() << '\n';

    Container moved(move(copy));
    cout << "  move construida size=" << moved.size() << '\n';

    Container copy2;
    copy2 = c;
    Container moved2;
    moved2 = move(copy2);
    cout << "  copy/move assignment size=" << moved2.size() << '\n';
}

#endif // DEMO_UTILS_H
