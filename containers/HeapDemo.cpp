#include <iostream>
#include <fstream>
#include <tuple>

#include "../types.h"
#include "heap.h"

using namespace std;

void DemoMinHeap() {
    cout << "--- MinHeap ---" << endl;

    Heap<MinHeapTrait<T1>> heap;
    for (T1 valor : {5, 3, 8, 1, 9, 2, 7})
        heap.insert(valor, valor * 10);

    cout << "arreglo : " << heap << endl;
    cout << "size    : " << heap.size() << endl;

    auto [cima, ref] = heap.peek();
    cout << "peek    : (" << cima << "," << ref << ")" << endl;

    cout << "extract : ";
    while (!heap.isEmpty()) {
        auto [dato, refDato] = heap.extract();
        cout << "(" << dato << "," << refDato << ") ";
    }
    cout << endl;
}

void DemoMaxHeap() {
    cout << "--- MaxHeap ---" << endl;

    Heap<MaxHeapTrait<T1>> heap;
    for (T1 valor : {5, 3, 8, 1, 9, 2, 7})
        heap.insert(valor, valor * 10);

    cout << "arreglo : " << heap << endl;

    auto [cima, ref] = heap.peek();
    cout << "peek    : (" << cima << "," << ref << ")" << endl;

    cout << "extract : ";
    while (!heap.isEmpty()) {
        auto [dato, refDato] = heap.extract();
        cout << "(" << dato << "," << refDato << ") ";
    }
    cout << endl;

    // Guardar y volver a leer desde archivo
    Heap<MaxHeapTrait<T1>> original;
    for (T1 valor : {4, 10, 6})
        original.insert(valor, valor);

    ofstream salida("temp.txt");
    salida << original << endl;
    salida.close();

    Heap<MaxHeapTrait<T1>> leido;
    ifstream entrada("temp.txt");
    entrada >> leido;
    entrada.close();

    cout << "guardado: " << original << endl;
    cout << "leido   : " << leido << endl;
}
