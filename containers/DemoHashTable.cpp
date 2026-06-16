#include <iostream>
#include <string>
#include "../types.h"
#include "hashAVL.h"

using namespace std;

void DemoHashAVL() {
    cout << "       PRUEBAS HASHAVL        " << endl;

    HashAVL<Long, Cadena> m;

    // ── operator[] ──────────────────────────
    cout << "\n[1] Insercion con operator[]:" << endl;
    m[5]  = "Cinco";
    m[25] = "Veinticinco";
    m[5]  = "CincoModificado";   // actualiza valor existente
    m[3]  = "Tres";
    m[18] = "Dieciocho";

    cout << "  m[5]  = " << m[5]  << endl;
    cout << "  m[25] = " << m[25] << endl;
    cout << "  m[3]  = " << m[3]  << endl;

    // ── size ────────────────────────────────
    cout << "\n[2] Tamanio: " << m.size() << " elementos" << endl;

    // ── operator<< ──────────────────────────
    cout << "\n[3] operator<<:" << endl;
    cout << "  " << m << endl;

    // ── ForEach ─────────────────────────────
    cout << "\n[4] ForEach (inorder por hash):" << endl;
    m.ForEach([](HashAVLEntry<Long, Cadena>& e) {
        cout << "  Key: " << e.first << " -> Value: " << e.second << endl;
    });

    // ── Copia ───────────────────────────────
    cout << "\n[5] Constructor copia:" << endl;
    HashAVL<Long, Cadena> copia(m);
    copia[99] = "Noventa y nueve";
    cout << "  Tamanio original : " << m.size()    << endl;
    cout << "  Tamanio copia    : " << copia.size() << endl;
    cout << "  Original : " << m    << endl;
    cout << "  Copia    : " << copia << endl;

    // ── Move ────────────────────────────────
    cout << "\n[6] Move constructor:" << endl;
    HashAVL<Long, Cadena> movido(std::move(copia));
    cout << "  Tamanio movido : " << movido.size() << endl;
    cout << "  Tamanio copia tras move: " << copia.size() << " (vacio)" << endl;

    // ── operator>> ──────────────────────────
    cout << "\n[7] operator>> (deserializar):" << endl;
    HashAVL<Long, Cadena> desde_stream;
    // Formato: [(clave,valor),(clave,valor),...]
    istringstream iss("[(7,Siete),(42,Cuarenta),(1,Uno)]");
    iss >> desde_stream;
    cout << "  Leido: " << desde_stream << endl;

    cout << "      FIN PRUEBAS HASHAVL     " << endl;
}

