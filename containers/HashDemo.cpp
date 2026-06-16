#include <iostream>
#include <fstream>
#include <string>
#include "../types.h"
#include "hashtable.h"
using namespace std;

void HashDemo() {
    cout << "\n=== HashTable Demo ===" << endl;

    // armo el mapa de una
    HashTable<T1, string> m = {{1, "Geeks"}, {2, "For"}, {3, "Geeks"}};
    cout << "inicial: " << m << endl;

    // [] crea la key si no existe; at solo actualiza
    m[0]    = "Tweaks";
    m.at(1) = "By";
    cout << "m[0]    = " << m[0]    << endl;
    cout << "m.at(1) = " << m.at(1) << endl;
    cout << "tras updates: " << m << endl;

    // recorro en orden por key
    cout << "[key,value]: ";
    for(const auto& [k, v] : m) cout << "(" << k << "->" << v << ") ";
    cout << endl;

    cout << "contains(2): " << m.contains(2) << " | contains(9): " << m.contains(9) << endl;

    // guardo y vuelvo a leer del archivo
    ofstream osH("HASH.txt");
    osH << m << endl;
    osH.close();
    HashTable<T1, string> m2;
    ifstream isH("HASH.txt");
    isH >> m2;
    cout << "leida desde archivo: " << m2 << endl;

    // copia (no afecta al original)
    HashTable<T1, string> mCopia(m);
    mCopia[5] = "Copia";
    cout << "copia (+[5]=Copia): " << mCopia << endl;
    cout << "original intacto:   " << m << endl;

    // move
    HashTable<T1, string> mMovida(move(mCopia));
    cout << "movida: " << mMovida << endl;
}
