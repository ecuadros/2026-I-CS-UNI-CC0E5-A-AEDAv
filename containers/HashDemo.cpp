#include <iostream>
#include <fstream>
#include <string>
#include "../types.h"
#include "hashtable.h"
using namespace std;

// hash de prubea (k % 4) para forzar colisiones en la demo
struct DemoHash { size_t operator()(T1 k) const { return (size_t)(k % 4); } };

void HashDemo(ostream& os) {
    using Hash = HashTable<HashTrait<T1, string, DemoHash>>;
    os << "\n=== HashTable Demo ===" << endl;

    Hash m = {{1, "Geeks"}, {2, "For"}, {3, "Geeks"}};
    os << "inicial: " << m << endl;

    // [] crea la key si no existe. at solo actualiza
    m[0]    = "Tweaks";
    m.at(1) = "By";
    os << "m[0]    = " << m[0]    << endl;
    os << "m.at(1) = " << m.at(1) << endl;
    os << "tras updates: " << m << endl;

    // COLISION: 5 y 1 caen en el mismo bucket (5 % 4 == 1 % 4) -> misma cadena
    m[5] = "Five";
    os << "colision (hash de 1 y 5 = " << (5 % 4) << "): m[1]=" << m[1] << " | m[5]=" << m[5] << endl;
    os << "tras colision: " << m << endl;

    // recorrido de entradas (el iterador aplana buckets->cadena)
    os << "entradas: ";
    for(const auto& [k, v] : m) os << "(" << k << "->" << v << ") ";
    os << endl;

    os << "contains(5): " << m.contains(5) << " | contains(9): " << m.contains(9) << endl;

    // operator<< / operator>> en archivo
    ofstream osH("HASH.txt");
    osH << m << endl;
    osH.close();
    Hash m2;
    ifstream isH("HASH.txt");
    isH >> m2;
    os << "leida desde archivo: " << m2 << endl;

    // copy constructor
    Hash mCopia(m);
    mCopia[9] = "Copia";          // 9 % 4 == 1 -> colisiona con 1 y 5 en la copia
    os << "copia (+[9]=Copia): " << mCopia << endl;
    os << "original intacto:   " << m << endl;

    // move constructor
    Hash mMovida(move(mCopia));
    os << "movida: " << mMovida << endl;
}
