#include <iostream>
#include <string>
#include <cstddef>
#include <cctype>
#include "../types.h"
#include "BTree.h"
using namespace std;

using BT = BTree<AscendingBTreeTrait<char>>;

void BTreeDemo() {
    cout << "=== B-TREE (orden 3) ===\n";

    BT bt(3);
    string keys = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    for (size_t i = 0; i < keys.size(); i++)
        bt.Insert(keys[i], Ref(i * i));

    cout << "size=" << bt.size() << "  height=" << bt.height() << "  order=" << bt.GetOrder() << "\n\n";
    bt.Print(cout);

    // busqueda
    cout << "\nSearch('Z') -> " << bt.Search('Z') << "\n";
    cout << "Search('@') -> " << bt.Search('@') << " (no existe)\n";

    // ForEach variadico: contamos cuantas claves son letras
    size_t letras = 0;
    bt.ForEach([](BT::ObjectInfo& info, size_t, size_t& acc) {
        if (isalpha((unsigned char)info.key)) acc++;
    }, letras);
    cout << "letras en el arbol: " << letras << "\n";

    // FirstThat variadico: buscamos la primera clave igual a 'M'
    auto* hit = bt.FirstThat([](BT::ObjectInfo& info, size_t, char target) {
        return info.key == target;
    }, 'M');
    cout << "FirstThat('M') -> " << (hit ? "encontrado" : "no");
    if (hit) cout << ", ObjID=" << hit->ObjID;
    cout << "\n";

    // borramos algunas claves existentes
    bt.Remove('A', -1);
    bt.Remove('z', -1);
    bt.Remove('5', -1);
    cout << "tras borrar A,z,5 -> size=" << bt.size() << "\n";
}
