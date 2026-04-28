#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
#include "circulardoublelinkedlist.h"

using namespace std;

void LinkedListDemo()
{
    cout << "\nLinkedList Demo" << endl;

    LinkedList<AscendingLinkedListTrait<T1>> asc;
    asc.insert(28, 15);
    asc.insert(17, 25);
    asc.insert(8, 35);
    asc.insert(4, 45);
    asc.insert(35, 55);
    cout << "Ascendente:    " << asc << endl;

    ofstream os("AscLL.txt");
    os << asc;
    os.close();

    LinkedList<AscendingLinkedListTrait<T1>> from_file;
    ifstream is("AscLL.txt");
    is >> from_file;
    cout << "Desde archivo: " << from_file << endl;

    LinkedList<DescendingLinkedListTrait<T1>> desc;
    desc.insert(28, 15);
    desc.insert(17, 25);
    desc.insert(8, 35);
    cout << "Descendente:   " << desc << endl;
}

void DoubleLinkedListDemo()
{
    cout << "\nDoubleLinkedList Demo" << endl;

    DoubleLinkedList<AscendingDLLTrait<T1>> list;
    list.push_back(10, 1);
    list.push_back(20, 2);
    list.push_back(30, 3);
    list.push_front(5, 0);
    cout << "Ascendente (operator<<): " << list << endl;

    DoubleLinkedList<DescendingDLLTrait<T1>> listDesc;
    listDesc.push_back(30, 1);
    listDesc.push_back(20, 2);
    listDesc.push_back(10, 3);
    cout << "Descendente (operator<<): " << listDesc << endl;

    cout << "Foreach nativo: ";
    for (auto &x : list)
        cout << x << " ";
    cout << endl;

    cout << "Forward iterator: ";
    for (auto it = list.begin(); it != list.end(); ++it)
        cout << *it << " ";
    cout << endl;

    cout << "Backward iterator: ";
    for (auto it = list.rbegin(); it != list.rend(); ++it)
        cout << *it << " ";
    cout << endl;

    cout << "ForEach heredado: ";
    list.ForEach([](T1 &x)
                 { cout << x << " "; });
    cout << endl;

    DoubleLinkedList<AscendingDLLTrait<T1>> copy(list);
    cout << "Copy constructor: " << copy << endl;

    DoubleLinkedList<AscendingDLLTrait<T1>> copyAssigned;
    copyAssigned = list;
    cout << "Copy assignment:  " << copyAssigned << endl;

    DoubleLinkedList<AscendingDLLTrait<T1>> moved(std::move(copy));
    cout << "Move constructor: " << moved << endl;

    DoubleLinkedList<AscendingDLLTrait<T1>> moveAssigned;
    moveAssigned = std::move(moved);
    cout << "Move assignment:  " << moveAssigned << endl;

    ofstream os("DLL.txt");
    os << list;
    os.close();
    DoubleLinkedList<AscendingDLLTrait<T1>> loaded;
    ifstream is("DLL.txt");
    is >> loaded;
    cout << "operator>> archivo: " << loaded << endl;
}

void CircularLinkedListDemo()
{
    cout << "\nCircularLinkedList Demo" << endl;

    CircularLinkedList<AscendingCLLTrait<T1>> list;
    list.push_back(10, 1);
    list.push_back(20, 2);
    list.push_back(30, 3);
    list.push_front(5, 0);

    cout << "Recorrido: ";
    for (auto it = list.begin(); it != list.end(); ++it)
        cout << *it << " ";
    cout << endl;

    auto [val, ref] = list.pop_front();
    cout << "pop_front: (" << val << ", " << ref << ")" << endl;

    cout << "Tras pop:  ";
    for (auto it = list.begin(); it != list.end(); ++it)
        cout << *it << " ";
    cout << endl;
}

void CircularDoubleLinkedListDemo()
{
    cout << "\nCircularDoubleLinkedList Demo" << endl;

    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> list;
    list.push_back(10, 1);
    list.push_back(20, 2);
    list.push_back(30, 3);
    list.push_front(5, 0);

    cout << "Forward:  ";
    for (auto it = list.begin(); it != list.end(); ++it)
        cout << *it << " ";
    cout << endl;

    cout << "Backward: ";
    for (auto it = list.rbegin(); it != list.rend(); ++it)
        cout << *it << " ";
    cout << endl;
}

void TestConcurrencia()
{
    cout << "\nTest de Concurrencia" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;
    auto worker = [&list](int id)
    {
        for (int i = 0; i < 1000; i++)
            list.push_front(i, id);
    };
    thread t1(worker, 1), t2(worker, 2), t3(worker, 3), t4(worker, 4), t5(worker, 5);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    cout << "Tamano (esperado 5000): " << list.size() << endl;
    cout << (list.size() == 5000 ? "EXITO" : "FALLO") << endl;
}

void TestOperators()
{
    cout << "\nTest de Operadores" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;
    stringstream ss("[(10,100)(20,200)(30,300)]");
    ss >> list;
    cout << "operator<<:  " << list << endl;
    cout << "operator[0]: " << list[0] << endl;
    cout << "operator[2]: " << list[2] << endl;
}

void ListsDemo()
{
    LinkedListDemo();
    CircularLinkedListDemo();
    DoubleLinkedListDemo();
    CircularDoubleLinkedListDemo();
    TestConcurrencia();
    TestOperators();
    cout << "\nFIN" << endl;
}
