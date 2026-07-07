#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "../types.h"
#include "BTree.h"

using namespace std;

const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyzKLlmn";

const T1 BTreeOrder = 3;

void TestBasicos(){
    cout << "\nTEST BASICO (Insert/Print/Search/Remove)" << endl;
    BTree<BTreeTrait<char, long>> bt(BTreeOrder);

    for( T1 i = 0; keys1[i]; i++ )
        bt.Insert(keys1[i], i*i);

    cout << "Arbol tras insertar " << bt.size() << " claves:" << endl;
    bt.Print(cout);

    for( T1 i = 0; keys2[i]; i++ )
    {
        long ObjID = bt.Search(keys2[i]);
        if( ObjID != -1 )
            cout << "Encontre " << keys2[i] << " ID = " << ObjID << endl;
        else
            cout << "No encontre " << keys2[i] << endl;
    }

    for( T1 i = 0; keys3[i]; i++ )
    {
        if( bt.Remove(keys3[i], -1) )
            cout << keys3[i] << " removido!" << endl;
        else
            cout << "No encontre " << keys3[i] << endl;
    }

    cout << "Arbol tras remover (quedan " << bt.size() << " claves):" << endl;
    bt.Print(cout);
}

void TestIteradores(){
    cout << "\nTEST DE ITERADORES (forward/backward) Y ForEach/FirstThat" << endl;
    BTree<BTreeTrait<char, long>> bt(BTreeOrder);
    for( T1 i = 0; keys1[i]; i++ )
        bt.Insert(keys1[i], i*i);

    cout << "Forward  (ascendente): ";
    for( auto it = bt.begin(); it != bt.end(); ++it )
        cout << it->key;
    cout << endl;

    cout << "Backward (descendente): ";
    for( auto it = bt.rbegin(); it != bt.rend(); ++it )
        cout << it->key;
    cout << endl;

    T1 vocales = 0;
    bt.ForEach([](auto &info, T1 *pCount){
        if( string("AEIOUaeiou").find(info.key) != string::npos )
            (*pCount)++;
        return true; // seguir recorriendo
    }, &vocales);
    cout << "Cantidad de vocales en el arbol: " << vocales << endl;

    auto *primeraMayus = bt.FirstThat([](auto &info){
        return info.key >= 'A' && info.key <= 'Z';
    });
    if( primeraMayus )
        cout << "Primera mayuscula en orden ascendente: " << primeraMayus->key << endl;
}

void TestConcurrencia(){
    cout << "\nTEST DE CONCURRENCIA" << endl;
    BTree<BTreeTrait<T1>> bt(BTreeOrder);

    const T1 N_HILOS = 5;
    const T1 N_POR_HILO = 200;
    auto worker = [&bt](T1 hiloId){
        for( T1 i = 0; i < N_POR_HILO; i++ )
            bt.Insert(hiloId*N_POR_HILO + i, hiloId);
    };

    vector<thread> hilos;
    for( T1 h = 0; h < N_HILOS; h++ )
        hilos.emplace_back(worker, h);
    for( auto &h : hilos )
        h.join();

    cout << "Se lanzaron " << N_HILOS << " hilos insertando " << N_POR_HILO << " claves cada uno." << endl;
    cout << "Tamano del arbol (esperado " << N_HILOS*N_POR_HILO << "): " << bt.size() << endl;
    if( bt.size() == N_HILOS*N_POR_HILO )
        cout << "ESTADO: EXITO - El shared_mutex protegio el arbol correctamente." << endl;
    else
        cout << "ESTADO: FALLO - Hubo corrupcion de datos." << endl;
}

void TestOperadores(){
    cout << "\nTEST DE OPERADORES (<</>>)" << endl;
    BTree<BTreeTrait<T1>> bt(BTreeOrder);

    cout << "Simulando lectura desde formato: [(10,100),(20,200),(30,300)]" << endl;
    istringstream simuladorInput("[(10,100),(20,200),(30,300)]");
    simuladorInput >> bt;

    cout << "Arbol luego de la lectura (operator<<): " << bt << endl;
}

void BTreeDemo(){
    TestBasicos();
    TestIteradores();
    TestConcurrencia();
    TestOperadores();
    cout << "\n=== FIN DE LAS PRUEBAS DE BTree ===" << endl;
}
