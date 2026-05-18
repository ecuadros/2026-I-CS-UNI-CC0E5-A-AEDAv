#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "../types.h"
#include "circularlinkedlist.h"
#include "doublelinkedlist.h"
#include "linkedlist.h"

using namespace std;

template <typename Container>
void DemoList(Container &list, const string &fileName){
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
    DoubleLinkedList<AscendingDLLTrait<T1>> list;
    DemoList(list, "AscDLL.txt");

    DoubleLinkedList<DescendingDLLTrait<T1>> list2;
    DemoList(list2, "DescDLL.txt");
}

void CircularLinkedListDemo(){
    CircularLinkedList<AscendingCLLTrait<T1>> list;
    DemoList(list, "AscCLL.txt");

    CircularLinkedList<DescendingCLLTrait<T1>> list2;
    DemoList(list2, "DescCLL.txt");
}

void CircularDoubleLinkedListDemo(){
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> list;
    DemoList(list, "AscCDLL.txt");

    CircularDoubleLinkedList<DescendingDLLTrait<T1>> list2;
    DemoList(list2, "DescCDLL.txt");
}

void TestConcurrencia(){
    cout << "\nTEST DE CONCURRENCIA" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;

    auto worker = [&list](int thread_id){
        for(int i = 0; i < 1000; ++i){
            list.push_front(i, thread_id);
        }
    };

    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    cout << "Se lanzaron 5 hilos insertando 1000 elementos simultaneamente." << endl;
    cout << "Tamano de la lista (Esperado 5000): " << list.size() << endl;
}

void TestOperators(){
    cout << "\nTEST DE OPERADORES" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;
    stringstream simulador_input("[(10,100),(20,200),(30,300)]");
    simulador_input >> list;
    cout << "Lista luego de la lectura (operator<<): " << list << endl;
    cout << "Accediendo al indice [0] (operator[]): Dato -> " << list[0] << endl;
    cout << "Accediendo al indice [2] (operator[]): Dato -> " << list[2] << endl;
}

void ListsDemo(){
    LinkedListDemo();
    DoubleLinkedListDemo();
    CircularLinkedListDemo();
    CircularDoubleLinkedListDemo();
    TestConcurrencia();
    TestOperators();
    cout << "\n=== FIN DE LAS PRUEBAS ===" << endl;
}
