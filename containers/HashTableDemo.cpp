#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "hash_table.h"
#include "../types.h"

using namespace std;

void DemoHashTable() {
    cout << "\nDemo de HashTable\n";

    // 1. Instanciación
    HashTable<string, T1> inventario;
    cout << "1. Tabla creada exitosamente.\n";

    // 2. Inserción mediante insert()
    inventario.insert("Manzanas", 50);
    inventario.insert("Naranjas", 30);
    cout << "2. Datos insertados con insert():\n" << inventario << "\n\n";

    // 3.operator[] (Inserción y Actualización)
    inventario["Pares"] = 100; 
    inventario["Naranjas"] = 85; 
    cout << "3. Datos tras usar operator[] (Pares=100, Naranjas actualizado a 85):\n" 
         << inventario << "\n\n";
    
    cout << "4. Iteracion con el for nativo:\n";
    T1 total_frutas = 0;
    for (const auto& [fruta, cantidad] : inventario.inorder()) {
        cout << "   - Producto: " << fruta << " | Cantidad: " << cantidad << "\n";
        total_frutas += cantidad;
    }

    cout << "   -> Total de frutas en inventario: " << total_frutas << "\n\n";

    // 5. Copy Constructor
    cout << "5. Probando Copy Constructor:\n";
    HashTable<string, T1> inventario_respaldo = inventario;
    inventario_respaldo["Manzanas"] = 9999; 
    
    cout << "   Original: " << inventario << "\n";
    cout << "   Respaldo: " << inventario_respaldo << " (Manzanas fue modificado de forma segura)\n\n";

    // 6. Persistencia en .txt
    cout << "6. Probando persistencia en disco:\n";
    string filename = "inventario_bd.txt";
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << inventario;
        outFile.close();
        cout << "   -> Datos exportados exitosamente a '" << filename << "'\n";
    } else {
        cout << "   [Error] No se pudo crear el archivo.\n";
    }

    // 6.2 Leer desde el archivo >>
    HashTable<string, T1> inventario_importado;
    ifstream inFile(filename);
    if (inFile.is_open()) {
        inFile >> inventario_importado;
        inFile.close();
        cout << "   -> Datos importados y reconstruidos desde '" << filename << "':\n";
        cout << "   Tabla recuperada: " << inventario_importado << "\n";
    } else {
        cout << "   [Error] No se pudo leer el archivo.\n";
    }
    
    cout << "\nDemo finalizado sin errores de memoria.\n";
}
