#include <iostream>
#include <fstream>

#include "../types.h"
#include "hashtable.h"

using namespace std;

void DemoHashTable() {
    cout << "--- HashTable (sobre AVL) ---" << endl;

    HashTable<AscendingAVLTrait<T1>> tabla;
    tabla[5] = 3;
    tabla[2] = 20;
    tabla[8] = 80;
    tabla[1] = 10;
    tabla[5] = 33;   // la clave ya existe, se actualiza

    cout << "tabla[5] = " << tabla[5] << endl;
    cout << "size     = " << tabla.size() << ", height = " << tabla.height() << endl;

    cout << "recorrido: ";
    for (const auto& [clave, valor] : tabla)
        cout << clave << "=" << valor << " ";
    cout << endl;

    cout << "serializa: " << tabla << endl;

    // Guardar y volver a leer desde archivo
    ofstream salida("temp.txt");
    salida << tabla << endl;
    salida.close();

    HashTable<AscendingAVLTrait<T1>> leida;
    ifstream entrada("temp.txt");
    entrada >> leida;
    entrada.close();
    cout << "leida    : " << leida << endl;

    // La copia es independiente del original
    HashTable<AscendingAVLTrait<T1>> copia(tabla);
    copia[5] = 999;
    cout << "original tabla[5] = " << tabla[5] << ", copia[5] = " << copia[5] << endl;

    HashTable<AscendingAVLTrait<T1>> movida(move(leida));
    cout << "movida   : " << movida << endl;
}
