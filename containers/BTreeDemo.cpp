#include <iostream>
#include <string>
#include "../types.h"
#include "btree.h"
using namespace std;

void BTreeDemo(ostream& os) {
    os << "\n=== BTree Demo ===" << endl;

    BTree<AscendingBTreeTrait<T1>> bt(3);

    // conjunto desordenado. ref = i*i
    T1 claves[] = {50, 20, 70, 10, 30, 60, 80, 5, 15, 25, 35, 55, 65, 75, 85};
    size_t n = sizeof(claves) / sizeof(claves[0]);
    for(size_t i = 0; i < n; ++i)
        bt.insert(claves[i], (Ref)(i * i));

    os << "altura: " << bt.height() << " | claves: " << bt.size()
       << " | orden: " << bt.order() << endl;

    // toString / operator<< : el arbol inorder ("clave->ref")
    os << "arbol (inorder, sangria por nivel):" << endl;
    os << bt;

    // ForEach variadico: claves en orden ascendente
    os << "ForEach (inorder): ";
    bt.ForEach([&os](T1 key) { os << key << " "; });
    os << endl;

    // FirstThat variadico: primera clave que cumple el predicado
    auto [k, r] = bt.FirstThat([](T1 key) { return key > 50; });
    os << "FirstThat (> 50): clave=" << k << " ref=" << r << endl;

    // search (clave, ref)
    auto [sk, sr] = bt.search(35);
    os << "search(35): clave=" << sk << " ref=" << sr << endl;
}
