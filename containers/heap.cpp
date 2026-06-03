#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include "heap.h"

using namespace std;

template <typename HeapType>
void RunHeapDemo(const string& title, const string& fileName) {
    cout << "\n" << title << "\n";
    HeapType heap;

    heap.insert(T1(20), Ref(200));
    heap.insert(T1(10), Ref(100));
    heap.insert(T1(30), Ref(300));
    heap.insert(T1(15), Ref(150));

    cout << "Estado inicial: " << heap << "\n";
    auto top = heap.peek();
    cout << "peek(): (" << top.getData() << "," << top.getRef() << ")\n";

    auto ex = heap.extract();
    cout << "extract(): (" << ex.getData() << "," << ex.getRef() << ")\n";
    cout << "Luego de extract: " << heap << "\n";

    {
        ofstream os(fileName);
        os << heap;
    }

    HeapType loaded;
    {
        ifstream is(fileName);
        is >> loaded;
    }
    cout << "Cargado con operator>>: " << loaded << "\n";

    HeapType concurrentHeap;
    auto worker = [&concurrentHeap](Ref refSeed) {
        for (T1 i = 0; i < T1(100); ++i)
            concurrentHeap.insert(T1(i + refSeed), Ref(refSeed));
    };

    thread t1(worker, Ref(1));
    thread t2(worker, Ref(2));
    thread t3(worker, Ref(3));
    thread t4(worker, Ref(4));
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    cout << "Concurrencia -> size esperado 400, real: " << concurrentHeap.size() << "\n";
}

void DemoMinHeap() {
    using MinHeap = Heap<MinHeapTrait<T1>>;
    RunHeapDemo<MinHeap>("DemoMinHeap", "min_heap.txt");
}

void DemoMaxHeap() {
    using MaxHeap = Heap<MaxHeapTrait<T1>>;
    RunHeapDemo<MaxHeap>("DemoMaxHeap", "max_heap.txt");
}
