#include <iostream>
#include <fstream>

#include "../types.h"
#include "hashtable.h"

using namespace std;

void DemoHashTable(std::ostream&) {
    cout << "--- HashTable (funcion hash + colisiones resueltas con AVL) ---" << endl;

    HashTable<AscendingHashTrait<T1>> tabla(4);
    tabla[5]  = 50;
    tabla[2]  = 20;
    tabla[8]  = 80;
    tabla[1]  = 10;
    tabla[9]  = 90;
    tabla[13] = 130;
    tabla[5]  = 55;

    cout << "tabla[5]   = " << tabla[5] << endl;
    cout << "size       = " << tabla.size()
         << ", cubetas = " << tabla.bucketCount()
         << ", carga = "    << tabla.loadFactor() << endl;

    cout << "reparto por la funcion hash:" << endl;
    for (T1 k : {5, 2, 8, 1, 9, 13})
        cout << "  hash(" << k << ") -> cubeta " << tabla.indexFor(k) << endl;

    cout << "contenido de cada cubeta (cada una es un AVL):" << endl;
    for (size_t i = 0; i < tabla.bucketCount(); ++i)
        cout << "  cubeta[" << i << "] = " << tabla.bucketToString(i) << endl;

    cout << "recorrido  : ";
    for (const auto& [clave, valor] : tabla)
        cout << clave << "=" << valor << " ";
    cout << endl;

    cout << "contains(9)  = " << tabla.contains(9)
         << ", contains(7) = " << tabla.contains(7) << endl;

    cout << "serializa  : " << tabla << endl;

    ofstream salida("temp.txt");
    salida << tabla << endl;
    salida.close();

    HashTable<AscendingHashTrait<T1>> leida(4);
    ifstream entrada("temp.txt");
    entrada >> leida;
    entrada.close();
    cout << "leida      : " << leida << endl;

    HashTable<AscendingHashTrait<T1>> copia(tabla);
    copia[5] = 999;
    cout << "original tabla[5] = " << tabla[5] << ", copia[5] = " << copia[5] << endl;

    HashTable<AscendingHashTrait<T1>> movida(move(leida));
    cout << "movida     : " << movida << endl;
}
