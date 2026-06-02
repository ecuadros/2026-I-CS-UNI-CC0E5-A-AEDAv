#include <iostream>
#include <string>
#include "heap.h"
#include "../types.h"

using namespace std;

template <typename HeapType>
void DemoGenericHeap(HeapType& heap, const string& name) {
    cout << "\n--- " << name << " ---" << endl;
    
    cout << "1. Insertando 10, 50, 20, 70, 30, 80" << endl;
    heap.insert(10, 1);
    heap.insert(50, 2);
    heap.insert(20, 3);
    heap.insert(70, 4);
    heap.insert(30, 5);
    heap.insert(80, 6);
    
    cout << "   Tamaño: " << heap.size() << endl;
    cout << "   Estado en memoria (Vector subyacente): " << heap << endl;
    
    auto [p_data, p_ref] = heap.peek();
    cout << "2. Peek (Elemento en la raíz): (" << p_data << "," << p_ref << ")" << endl;
    
    cout << "3. Extrayendo elementos uno por uno (Orden de prioridad garantizado):" << endl;
    cout << "   ";
    while (!heap.isEmpty()) {
        auto [data, ref] = heap.extract();
        cout << "(" << data << "," << ref << ") ";
    }
    cout << "\n   Tamaño final tras extracciones: " << heap.size() << endl;
}

void HeapDemo() {
    // Inyectamos VectorNode<T1> en los Traits para decirle al Heap qué tipo de nodo procesar
    Heap<MinHeapTrait<T1>> minHeap;
    DemoGenericHeap(minHeap, "DEMO MIN-HEAP (Menor prioridad sale primero)");
    
    Heap<MaxHeapTrait<T1>> maxHeap;
    DemoGenericHeap(maxHeap, "DEMO MAX-HEAP (Mayor prioridad sale primero)");
}