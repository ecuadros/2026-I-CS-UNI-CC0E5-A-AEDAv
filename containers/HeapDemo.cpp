#include <iostream>
#include <initializer_list>
#include <tuple>
#include <utility>
#include "heap.h"
#include "../types.h"
using namespace std;

template <typename T>
void PrintHeapItem(const string &label, const tuple<T, Ref> &item) {
    cout << label << "(" << get<0>(item) << "," << get<1>(item) << ")" << endl;
}

template <typename Trait>
void InsertHeapItems(Heap<Trait> &heap, initializer_list<pair<typename Trait::value_type, Ref>> items) {
    for (const auto &[value, ref] : items) {
        heap.insert(value, ref);
    }
}

template <typename Trait>
void RunHeapDemo(const string &title) {
    cout << "\n" << title << endl;
    Heap<Trait> heap;
    InsertHeapItems(heap, {{47, 9031}, {-12, 418}, {86, 1207}, {7, 7764}, {31, 59}, {-4, 6402}});
    cout << heap << endl;
    PrintHeapItem("peek: ", heap.peek());
    PrintHeapItem("extract: ", heap.extract());
    cout << heap << endl;
}

void DemoMinHeap() {
    RunHeapDemo<MinHeapTrait<T1>>("Demo MinHeap");
}

void DemoMaxHeap() {
    RunHeapDemo<MaxHeapTrait<T1>>("Demo MaxHeap");
}

void HeapDemo() {
    DemoMinHeap();
    DemoMaxHeap();
}
