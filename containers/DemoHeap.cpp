#include <iostream>
#include <fstream>
#include <thread>
#include "../types.h"
#include "heap.h"
using namespace std;


using MaxHeap = Heap<DescendingTrait<HeapNode<T1>>>;
using MinHeap = Heap<AscendingTrait<HeapNode<T1>>>;

static const initializer_list<T1> kTestValues = {100, 414, 25, 1, 7, 45, 3};

void DemoMinHeap() {
    cout<<"\nMinHeap"<<endl;
    MinHeap h;

    //insert heapifyUp
    cout<<"insert:"<<endl;
    for (T1 v : kTestValues) {
        h.insert(v, v * 8);
        cout<<"insert("<<v<<")\n"<<h.toString();
    }

    // peek
    auto [pval, pref] = h.peek();
    cout<<"peek(minimo): "<<pval<<" ref:"<<pref<<endl;

    //operator<<
    { ofstream os("minHeap.txt"); os<<h; }
    cout<<"operator<<: "<<h<<endl;

    // operator>>
    MinHeap h2;
    { ifstream is("minHeap.txt"); is >> h2; }
    cout<<"operator>> leido: "<<h2<<endl;

    //forEach
    cout<<"forEach:  ";
    h.forEach([](MinHeap::value_type& v){ cout<<v<<" "; });
    cout<<endl;

    //range-based for
    cout<<"range-for: ";
    for (auto& node : h) cout<<node.m_data<<" ";
    cout<<endl;

    //extract heapifyDown
    cout<<"\nextract (orden ascendente):"<<endl;
    while (!h.isEmpty()) {
        auto [val, ref] = h.extract();
        cout<<"  extract -> "<<val<<" | ";
        h.forEach([](MinHeap::value_type& v){ cout<<v<<" "; });
        cout<<endl;
    }
}

void DemoMaxHeap() {
    cout<<"\nMaxHeap"<<endl;
    MaxHeap h;

    cout<<"insert:"<<endl;
    for (T1 v : kTestValues) {
        h.insert(v, v * 10);
        cout<<"insert("<<v<<")\n"<<h.toString();
    }

    auto [pval, pref] = h.peek();
    cout<<"peek (maximo): "<<pval<<" ref:"<<pref<<endl;

    // forEach
    cout<<"forEach:  ";
    h.forEach([](MaxHeap::value_type& v){ cout<<v<<" "; });
    cout<<endl;

    // range-based for
    cout<<"range-for: ";
    for (auto& node : h) cout<<node.m_data<<" ";
    cout<<endl;

    cout<<"\nextract descendente:"<<endl;
    while (!h.isEmpty()) {
        auto [val, ref] = h.extract();
        cout<<"  extract -> "<<val<<" | ";
        h.forEach([](MaxHeap::value_type& v){ cout<<v<<" "; });
        cout<<endl;
    }
}

void DemoHeapConcurrencia() {
    cout<<"\nconcurrencia"<<endl;
    MinHeap h;
    auto worker = [&h](MinHeap::value_type id) {
        MinHeap::value_type i{};
        while (i < 200) { h.insert(i * id, id); ++i; }
    };
    thread th1(worker,1), th2(worker,2), th3(worker,3),
           th4(worker,4), th5(worker,5);
    th1.join(); th2.join(); th3.join(); th4.join(); th5.join();
    cout<<"size 1000: "<<h.size()<<endl;
    auto [val, ref] = h.peek();
    cout<<"peek minimo: "<<val<<endl;
}

void HeapDemo() {
    cout<<endl;
    cout<<"PRUEBAS HEAP"<<endl;
    DemoMinHeap();
    DemoMaxHeap();
    DemoHeapConcurrencia();
    cout<<"\nFIN DE LAS PRUEBAS"<<endl;
}   