#include <fstream>
#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "../types.h"
#include "circularlinkedlist.h"
#include "doublelinkedlist.h"
#include "linkedlist.h"

using namespace std;

template <typename Container>
void DemoList(Container& list, const string& fileName) {
    list.insert(25, 1);
    list.insert(67, 2);
    list.insert(7, 3);
    list.insert(4, 4);
    list.insert(15, 5);
    cout << fileName << " -> " << list << endl;

    ofstream os(fileName);
    os << list << endl;

    Container copy;
    ifstream is(fileName);
    is >> copy;
    cout << fileName << " leido -> " << copy << endl;
}

// LL Circular, DoubleLinkedList y CDLL
void TestBasicos() {
    cout << "\nPruebas minimas de listas" << endl;

    LinkedList<AscendingLinkedListTrait<T1>> ascLL;
    DemoList(ascLL, "AscLL.txt");

    LinkedList<DescendingLinkedListTrait<T1>> descLL;
    DemoList(descLL, "DescLL.txt");

    CircularLinkedList<AscendingLinkedListTrait<T1>> ascCLL;
    DemoList(ascCLL, "AscCLL.txt");

    DoubleLinkedList<AscendingDLLTrait<T1>> ascDLL;
    DemoList(ascDLL, "AscDLL.txt");

    CircularDoubleLinkedList<AscendingDLLTrait<T1>> ascCDLL;
    DemoList(ascCDLL, "AscCDLL.txt");
    cout << "Validacion enlaces CDLL: "
         << (ascCDLL.validate_links() ? "OK" : "FALLO") << endl;

    cout << "Recorrido inverso DLL: ";
    for (auto it = ascDLL.rbegin(); it != ascDLL.rend(); ++it) {
        cout << *it << " ";
    }
    cout << endl;
}

void TestConcurrencia() {
    cout << "\nPrueba de concurrencia" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;
    mutex cout_mtx;

    // Demo de concurrencia
    auto worker = [&list, &cout_mtx](int thread_id) {
        {
            lock_guard<mutex> lock(cout_mtx);
            cout << "Hilo " << thread_id << " inicia" << endl;
        }

        for (int i = 0; i < 1000; ++i) {
            list.push_front(i, thread_id);

            if (i % 250 == 0) {
                lock_guard<mutex> lock(cout_mtx);
                cout << "Hilo " << thread_id << " inserto " << i << " elementos" << endl;
            }

            this_thread::sleep_for(chrono::milliseconds(1));
        }

        {
            lock_guard<mutex> lock(cout_mtx);
            cout << "Hilo " << thread_id << " termina" << endl;
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

    cout << "Tamano de la lista (Esperado 5000): " << list.size() << endl;
}

void TestOperators() {
    cout << "\nPrueba de los operadores" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;

    stringstream simulador_input("[(10,100),(20,200),(30,300)]");
    simulador_input >> list;

    //operator<> y operator>>
    cout << "Lista luego de operator>>: " << list << endl;
    cout << "operator[] indice 0: " << list[0] << endl;
    cout << "operator[] indice 2: " << list[2] << endl;
}

void ListsDemo() {
    TestBasicos();
    TestConcurrencia();
    TestOperators();
}
