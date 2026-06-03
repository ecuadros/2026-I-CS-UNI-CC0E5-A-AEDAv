#include <iostream>
#include <fstream>
#include <tuple>
#include "../types.h"
#include "heap.h"
using namespace std;

void DemoMinHeap() {
    cout << "\n=== MinHeap Demo ===" << endl;
    Heap<MinHeapTrait<T1>> h;
    for(auto [v, r] : { tuple<T1,Ref>{5,50}, {3,30}, {8,80}, {1,10}, {4,40}, {7,70} })
        h.insert(v, r);

    cout << "heap:  " << h << endl;
    auto [pd, pr] = h.peek();
    cout << "peek -> dato: " << pd << " ref: " << pr << endl;

    cout << "extract (ascendente): ";
    while(!h.isEmpty()) {
        auto [d, r] = h.extract();
        cout << "(" << d << "," << r << ") ";
    }
    cout << endl;

    // persistencia round-trip
    Heap<MinHeapTrait<T1>> h2;
    h2.insert(9, 90); h2.insert(2, 20); h2.insert(6, 60);
    ofstream os("HEAP.txt"); os << h2 << endl;
    Heap<MinHeapTrait<T1>> h3;
    ifstream is("HEAP.txt"); is >> h3;
    cout << "leido desde archivo: " << h3 << endl;
}

void DemoMaxHeap() {
    cout << "\n=== MaxHeap Demo ===" << endl;
    Heap<MaxHeapTrait<T1>> h;
    for(auto [v, r] : { tuple<T1,Ref>{5,50}, {3,30}, {8,80}, {1,10}, {4,40}, {7,70} })
        h.insert(v, r);

    cout << "heap:  " << h << endl;
    auto [pd, pr] = h.peek();
    cout << "peek -> dato: " << pd << " ref: " << pr << endl;

    cout << "extract (descendente): ";
    while(!h.isEmpty()) {
        auto [d, r] = h.extract();
        cout << "(" << d << "," << r << ") ";
    }
    cout << endl;
}
