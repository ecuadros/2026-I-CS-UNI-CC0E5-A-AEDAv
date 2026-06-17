#include <iostream>
#include <fstream>
#include <string>
#include "../types.h"
#include "avlmap.h"
using namespace std;

// escribe a 'os' (no a cout) para poder redirigir la salida
void AVLMapDemo(ostream& os) {
    os << "\n=== AVLMap Demo ===" << endl;

    AVLMap<T1, string> m = {{1, "Geeks"}, {2, "For"}, {3, "Geeks"}};
    os << "inicial: " << m << endl;

    // [] crea la key si no existe. at solo actualiza
    m[0]    = "Tweaks";
    m.at(1) = "By";
    os << "m[0]    = " << m[0]    << endl;
    os << "m.at(1) = " << m.at(1) << endl;
    os << "tras updates: " << m << endl;

    // recorrer en orden por key
    os << "[key,value]: ";
    for(const auto& [k, v] : m) os << "(" << k << "->" << v << ") ";
    os << endl;

    os << "contains(2): " << m.contains(2) << " | contains(9): " << m.contains(9) << endl;

    // guardar y volver a leer del archivo
    ofstream osM("AVLMAP.txt");
    osM << m << endl;
    osM.close();
    AVLMap<T1, string> m2;
    ifstream isM("AVLMAP.txt");
    isM >> m2;
    os << "leida desde archivo: " << m2 << endl;

    // copia
    AVLMap<T1, string> mCopia(m);
    mCopia[5] = "Copia";
    os << "copia (+[5]=Copia): " << mCopia << endl;
    os << "original intacto:   " << m << endl;

    // move
    AVLMap<T1, string> mMovida(move(mCopia));
    os << "movida: " << mMovida << endl;
}
