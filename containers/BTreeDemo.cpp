#include <iostream>
#include <fstream>
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

    // operator<< : [(clave,ref),...] en inorder
    os << "operator<<: " << bt << endl;

    // range-based for (forward)
    os << "for(auto& n : bt) (inorder): ";
    for(const auto& node : bt) os << node.getData() << " ";
    os << endl;

    // range-based for con la vista inversa
    os << "for(auto& n : bt.reversed()) (inverso): ";
    for(const auto& node : bt.reversed()) os << node.getData() << " ";
    os << endl;

    // ReverseForEach (backward)
    os << "ReverseForEach (inverso): ";
    bt.ReverseForEach([&os](auto& node) { os << node.getData() << " "; });
    os << endl;

    // FirstThat variadico: primer nodo que cumple el predicado
    auto [k, r] = bt.FirstThat([](auto& node) { return node.getData() > 50; });
    os << "FirstThat (> 50): clave=" << k << " ref=" << r << endl;

    // search (clave, ref)
    auto [sk, sr] = bt.search(35);
    os << "search(35): clave=" << sk << " ref=" << sr << endl;

    // operator>> : round-trip a archivo
    ofstream out("BTREE.txt");
    out << bt;
    out.close();
    BTree<AscendingBTreeTrait<T1>> bt2(3);
    ifstream in("BTREE.txt");
    in >> bt2;
    in.close();
    os << "leido de archivo: " << bt2 << endl;

    // comparador flexible: mismo codigo, orden invertido segun el Trait
    BTree<DescendingBTreeTrait<T1>> btDesc(3);
    for(size_t i = 0; i < n; ++i)
        btDesc.insert(claves[i], (Ref)(i * i));
    os << "descendente (DescendingBTreeTrait): " << btDesc << endl;

    // remove: borra varias claves y muestra que el arbol sigue ordenado
    bt.remove(50); bt.remove(10); bt.remove(85); bt.remove(35);
    os << "tras remove(50,10,85,35): " << bt << endl;
    os << "altura: " << bt.height() << " | claves: " << bt.size() << endl;
    os << "contiene 50? " << (bt.remove(50) ? "si (borrada de nuevo?!)" : "no (ya no esta)") << endl;
}
