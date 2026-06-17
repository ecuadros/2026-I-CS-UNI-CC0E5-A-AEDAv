#include <iostream>
#include <sstream>
#include <string>

#include "HashAVL.h"
#include "../types.h"

using namespace std;

// Trait para el ejemplo (puedes usar KeyValueTrait también)


void DemoHashTable() {
    cout << "PRUEBAS HASHTABLE (AVL)" << endl;

    HashTable<KeyValueTrait<Long, Cadena>> m;

    // ──────────────────────────────────────────
    cout << "\n[1] Insercion con operator[]:" << endl;

    m[5]  = "Cinco";
    m[25] = "Veinticinco";
    m[5]  = "CincoModificado";
    m[3]  = "Tres";
    m[18] = "Dieciocho";

    cout << "m[5]  = " << m[5] << endl;
    cout << "m[25] = " << m[25] << endl;
    cout << "m[3]  = " << m[3] << endl;

    // ──────────────────────────────────────────
    cout << "\n[2] Insercion con insert():" << endl;

    m.insert(100, "Cien");
    m.insert(200, "Doscientos");

    cout << "Contenido: " << m << endl;

    // ──────────────────────────────────────────
    cout << "\n[3] Recorrido inorder():" << endl;

    for (const auto& [key, value] : m.inorder()) {
        cout << "Key: " << key << " -> Value: " << value << endl;
    }

    // ──────────────────────────────────────────
    cout << "\n[4] Constructor copia:" << endl;

    HashTable<KeyValueTrait<Long, Cadena>> copia(m);
    copia[999] = "Novecientos noventa y nueve";

    cout << "Original: " << m << endl;
    cout << "Copia   : " << copia << endl;

    // ──────────────────────────────────────────
    cout << "\n[5] Move constructor / move assignment:" << endl;

    HashTable<KeyValueTrait<Long, Cadena>> movido(std::move(copia));

    cout << "Movido : " << movido << endl;
    cout << "Copia tras move: " << copia << endl;   // debería estar vacío

    // ──────────────────────────────────────────
    cout << "\n[6] operator>> (lectura desde stream):" << endl;

    HashTable<KeyValueTrait<Long, Cadena>> desde_stream;
    istringstream iss("{7:Siete, 42:CuarentaYDos, 1:Uno, 18:Dieciocho}");

    iss >> desde_stream;

    cout << "Leido del stream: " << desde_stream << endl;

    // ──────────────────────────────────────────
    cout << "\n[7] Verificacion de orden AVL (inorder):" << endl;

    for (const auto& [key, value] : desde_stream.inorder()) {
        cout << key << " -> " << value << endl;
    }

    cout << "      FIN PRUEBAS HASHTABLE" << endl;

}