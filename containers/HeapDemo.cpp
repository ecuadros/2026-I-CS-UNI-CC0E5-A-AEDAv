#include <iostream>
#include <sstream>
#include <tuple>
#include "heap.h"
#include "../types.h"
using namespace std;

template <typename T>
void PrintHeapItem(const string &label, const tuple<T, Ref> &item) {
    cout << label << "(" << get<0>(item) << "," << get<1>(item) << ")" << endl;
}

void DemoMinHeap() {
    cout << "\nDemo MinHeap" << endl;
    Heap<MinHeapTrait<T1>> heap;

    heap.insert(47, 9031);
    heap.insert(-12, 418);
    heap.insert(86, 1207);
    heap.insert(7, 7764);
    heap.insert(31, 59);
    heap.insert(-4, 6402);

    cout << heap << endl;
    PrintHeapItem("peek: ", heap.peek());
    PrintHeapItem("extract: ", heap.extract());
    cout << heap << endl;
}

void DemoMaxHeap() {
    cout << "\nDemo MaxHeap" << endl;
    Heap<MaxHeapTrait<T1>> heap;

    stringstream input("[(47,9031),(-12,418),(86,1207),(7,7764),(31,59),(-4,6402)]");
    input >> heap;

    cout << heap << endl;
    PrintHeapItem("peek: ", heap.peek());
    PrintHeapItem("extract: ", heap.extract());
    cout << heap << endl;
}

void HeapDemo() {
    DemoMinHeap();
    DemoMaxHeap();
}
