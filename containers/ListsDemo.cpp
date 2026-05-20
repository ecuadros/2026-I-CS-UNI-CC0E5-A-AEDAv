#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
#include "circulardoublelinkedlist.h"

using namespace std;

template <typename Container>
void DemoList(Container& list, string fileName){
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8, 35);
    list.insert(4, 45);
    list.insert(35, 55);
    cout << list << endl;
    ofstream os(fileName);
    os << list << endl;
    ifstream is(fileName);
    is >> list;
    cout << list << endl;
}

void LinkedListDemo(){
    LinkedList<AscendingLinkedListTrait<T1>> list;
    DemoList(list, "AscLL.txt");
    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");
}

void DoubleLinkedListDemo(){
    cout << "\n=== DoubleLinkedList Demo ===" << endl;

    DoubleLinkedList<AscendingDLLTrait<T1>> list;
    list.insert(3, 30);
    list.insert(1, 10);
    list.insert(5, 50);
    list.insert(2, 20);
    cout << "insert asc:     " << list << endl;

    // Bucle nativo forward
    cout << "for (forward):  ";
    for(auto& item : list) cout << item << " ";
    cout << endl;

    // ReverseForEach (backward iterator)
    cout << "ReverseForEach: ";
    list.ReverseForEach([](auto& item){ cout << item << " "; });
    cout << endl;

    // push_front y push_back
    list.push_front(0, 0);
    list.push_back(9, 90);
    cout << "push_front(0) push_back(9): " << list << endl;

    // pop_front y pop_back (retornan tuple)
    auto [d1, r1] = list.pop_front();
    auto [d2, r2] = list.pop_back();
    cout << "pop_front -> (" << d1 << "," << r1 << ")  pop_back -> (" << d2 << "," << r2 << ")" << endl;
    cout << "lista tras pops: " << list << endl;

    // Copy constructor
    DoubleLinkedList<AscendingDLLTrait<T1>> copia(list);
    cout << "copia:          " << copia << endl;

    // Move constructor
    DoubleLinkedList<AscendingDLLTrait<T1>> movida(move(list));
    cout << "movida:         " << movida << endl;
    cout << "original vacio: " << list << endl;

    // operator>> / operator<< con archivo
    ofstream os("DLL.txt");
    os << movida << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> leida;
    ifstream is2("DLL.txt");
    is2 >> leida;
    cout << "leida desde archivo: " << leida << endl;
}

void CircularLinkedListDemo(){
    cout << "\n=== CircularLinkedList Demo ===" << endl;

    CircularLinkedList<AscendingCLLTrait<T1>> list;
    list.insert(3, 30);
    list.insert(1, 10);
    list.insert(5, 50);
    list.insert(2, 20);
    cout << "insert asc:       " << list << endl;

    list.push_front(0, 0);
    cout << "push_front(0):    " << list << endl;

    list.push_back(9, 90);
    cout << "push_back(9):     " << list << endl;

    auto [d1, r1] = list.pop_front();
    cout << "pop_front -> (" << d1 << "," << r1 << ")  lista: " << list << endl;

    auto [d2, r2] = list.pop_back();
    cout << "pop_back  -> (" << d2 << "," << r2 << ")  lista: " << list << endl;

    // circularForEach: 2 vueltas completas
    cout << "circularForEach x2: ";
    list.circularForEach(2, [](auto& item){ cout << item << " "; });
    cout << endl;

    // operator>> / operator<< con archivo
    ofstream os("CLL.txt");
    os << list << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> leida;
    ifstream is2("CLL.txt");
    is2 >> leida;
    cout << "leida desde archivo: " << leida << endl;
}

void CircularDoubleLinkedListDemo(){
    cout << "\n=== CircularDoubleLinkedList Demo ===" << endl;

    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> list;
    list.insert(3, 30);
    list.insert(1, 10);
    list.insert(5, 50);
    list.insert(2, 20);
    cout << "insert asc: " << list << endl;

    list.push_front(0, 0);
    cout << "push_front: " << list << endl;

    list.push_back(9, 90);
    cout << "push_back:  " << list << endl;

    auto [d1, r1] = list.pop_front();
    cout << "pop_front -> (" << d1 << "," << r1 << "): " << list << endl;

    auto [d2, r2] = list.pop_back();
    cout << "pop_back  -> (" << d2 << "," << r2 << "): " << list << endl;

    // ForEach forward
    cout << "ForEach:        ";
    list.ForEach([](auto& item){ cout << item << " "; });
    cout << endl;

    // ReverseForEach backward circular
    cout << "ReverseForEach:      ";
    list.ReverseForEach([](auto& item){ cout << item << " "; });
    cout << endl;

    // circularForEach: 2 vueltas forward y 1 vuelta backward
    cout << "circularForEach fwd x2: ";
    list.circularForEach(2, 1, [](auto& item){ cout << item << " "; });
    cout << endl;
    cout << "circularForEach bwd x1: ";
    list.circularForEach(1, -1, [](auto& item){ cout << item << " "; });
    cout << endl;

    // operator>> / operator<< con archivo
    ofstream os("CDLL.txt");
    os << list << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> leida;
    ifstream is2("CDLL.txt");
    is2 >> leida;
    cout << "leida desde archivo: " << leida << endl;
}

void ListsDemo(){
    LinkedListDemo();
    DoubleLinkedListDemo();
    CircularLinkedListDemo();
    CircularDoubleLinkedListDemo();
}
