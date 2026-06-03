#include <iostream>
#include <string>
#include <fstream>
#include "../types.h"
#include "hashtable.h"

using namespace std;

void DemoHashTable() {
    cout<<"PRUEBAS HASHTABLE"<<endl;
    
    HashTable<long, string> m(3); 
    //operator[]
    m[10] = "Diez";
    m[25] = "Veinticinco";
    m[10] = "Diez Modificado"; 
    m[3]  = "Tres";

    cout<<"m[10] = "<<m[10]<<endl;

    //for (const auto& [key, value] : m)
    cout<<"\nfor-range:"<<endl;
    for (const auto& [key, value] : m) {
        cout<<"  Key: "<<key<<" -> Value: "<<value<<endl;
    }

    //operator<< y toString
    cout<<"\noperator<< (toString):"<<endl;
    cout<<m<<endl;

    //copia y movimiento
    HashTable<long, string> copia(m);
    copia[99] = "Dato de Copia";
    cout<<"\nTamano Original: "<<m.size()<<" | Tamano Copia: "<<copia.size()<<endl;

    cout<<"\nFIN PRUEBAS HASHTABLE"<<endl;
}