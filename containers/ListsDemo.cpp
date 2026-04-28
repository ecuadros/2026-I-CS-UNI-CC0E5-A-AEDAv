#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
// #include "circulardoublelinkedlist.h"

using namespace std;

template <typename Container>
void DemoList(Container& list, string fileName){
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8, 35);
    list.insert(4, 45);
    list.insert(35, 55);
    cout << "Lista Original: " << list << endl;
    ofstream os(fileName);
    os << list << endl;
    os.close();
    Container listFromFile;
    ifstream is(fileName);
    is >> listFromFile;
    cout << "Lista Leida de Archivo: " << listFromFile << endl;
}

void LinkedListDemo(){
    
    LinkedList<DescendingLinkedListTrait<T1>> list;
    list.insert(2, 200);
    list.insert(1, 100);
    list.insert(3, 300);
    
    LinkedList<DescendingLinkedListTrait<T1>> listMove(std::move(list));
    auto [data_front, ref_front] = listMove.pop_front();
    cout << "[POP FRONT EXITOSO] Dato: " << data_front << " | Metadato (Ref): " << ref_front << endl;

    cout << "\nTest de concurrencia" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> listconc;
    auto worker = [&listconc](int thread_id) {
        for(int i = 0; i < 1000; i++)
            listconc.push_front(i, thread_id);
    }; 
    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "Se lanzaron 5 hilos insertando 1000 elementos simultaneamente." << endl;
    cout << "Tamano de la lista (Esperado 5000): " << listconc.size() << endl;
    if(listconc.size() == 5000) {
        cout << "Estado: EXITO - El shared_mutex previno condiciones de carrera." << endl;
    } else {
        cout << "Estado: FALLO - Hubo corrupcion de memoria." << endl;
    }


    cout << "\nEscritura/Lectura archivos" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list1;
    DemoList(list1, "LLAsc.txt");
    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "LLDesc.txt");

    cout << "\nTest operadores" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> listOp;
    cout << "Simulando lectura: [(10, 100), (20, 200), (30, 300)]" << endl;
    stringstream simulador_input("[(10, 100), (20, 200), (30, 300)]");
    simulador_input >> listOp;
    cout << "Lista luego de la lectura (operator<<): " << listOp << endl;
    cout << "\nOperador[]: " << endl;
    cout << "Indice [0] (operator[]): Dato -> " << listOp[0] << endl;
    cout << "Indice [2] (operator[]): Dato -> " << listOp[2] << endl;
}

void DoubleLinkedListDemo(){
    DoubleLinkedList<AscendingDLLTrait<T1>> list;
    list.insert(10, 100);
    list.insert(20, 200);
    list.insert(30, 300);
    cout << "Lista Original: " << list << endl;

    cout << "\nTest de push/pop" << endl;
    list.push_back(40, 400);
    list.push_front(5, 50);
    cout << "Lista luego de push_back(40, 400) y push_front(5, 50): " << list << endl;
    auto [data_back, ref_back] = list.pop_back();
    cout << "Pop Back - Dato: " << data_back << " | Metadato (Ref): " << ref_back << endl;
    auto [data_front, ref_front] = list.pop_front();
    cout << "Pop Front - Dato: " << data_front << " | Metadato (Ref): " << ref_front << endl;

    cout << "\nTest de concurrencia" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> listconc;
    auto worker = [&listconc](int thread_id) {
        for(int i = 0; i < 1000; i++)
            listconc.push_back(i, thread_id);
    };
    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "Se lanzaron 5 hilos insertando 1000 elementos simultaneamente." << endl;
    cout << "Tamano de la lista (Esperado 5000): " << listconc.size() << endl;

    if(listconc.size() == 5000)
        cout << "Estado: EXITO - El shared_mutex previno condiciones de carrera." << endl;
    else
        cout << "Estado: FALLO - Hubo corrupcion de memoria." << endl;

    cout << "\nEscritura/Lectura archivos" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> list1;
    DemoList(list1, "DLLAsc.txt");
    DoubleLinkedList<DescendingDLLTrait<T1>> list2;
    DemoList(list2, "DLLDesc.txt");

    cout << "\nTest operadores" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> listOp;
    cout << "Simulando lectura: [(10, 100), (20, 200), (30, 300)]" << endl;
    stringstream simulador_input("[(10, 100), (20, 200), (30, 300)]");
    simulador_input >> listOp;
    cout << "Lista luego de la lectura (operator<<): " << listOp << endl;

    cout << "\nOperador[]: " << endl;
    cout << "Indice [0] (operator[]): Dato -> " << listOp[0] << endl;
    cout << "Indice [2] (operator[]): Dato -> " << listOp[2] << endl;

    cout << "\nTest iteradores" << endl;
    cout << "Forward iterator: ";
    for (auto &v : listOp)
        cout << "(" << v << ") ";
    cout << "\nBackward iterator: ";
    for (auto it = listOp.rbegin(); it != listOp.rend(); ++it)
        cout << "(" << *it << ") ";
     cout << endl;

}
void CircularLinkedListDemo(){
    CircularLinkedList<AscendingCLLTrait<T1>> list;
    list.insert(10, 100);
    list.insert(20, 200);
    list.insert(30, 300);
    cout << "Lista Original: " << list << endl;

    cout << "\nTest de push/pop" << endl;
    list.push_back(40, 400);
    list.push_front(5, 50);
    cout << "Lista luego de push_back(40, 400) y push_front(5, 50): " << list << endl;
    auto [data_back, ref_back] = list.pop_back();
    cout << "Pop Back - Dato: " << data_back << " | Metadato (Ref): " << ref_back << endl;
    auto [data_front, ref_front] = list.pop_front();
    cout << "Pop Front - Dato: " << data_front << " | Metadato (Ref): " << ref_front << endl;

    cout << "\nTest de concurrencia" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> listconc;
    auto worker = [&listconc](int thread_id) {
        for(int i = 0; i < 1000; i++)            listconc.push_back(i, thread_id);
    };
    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "Se lanzaron 5 hilos insertando 1000 elementos simultaneamente." << endl;
    cout << "Tamano de la lista (Esperado 5000): " << listconc.size() << endl;
    if(listconc.size() == 5000)
        cout << "Estado: EXITO - El shared_mutex previno condiciones de carrera." << endl;
    else
        cout << "Estado: FALLO - Hubo corrupcion de memoria." << endl;

    cout << "\nEscritura/Lectura archivos" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> list1;
    DemoList(list1, "CLLAsc.txt");
    CircularLinkedList<DescendingCLLTrait<T1>> list2;
    DemoList(list2, "CLLDesc.txt");

    cout << "\nTest operadores" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> listOp;
    cout << "Simulando lectura: [(10, 100), (20, 200), (30, 300)]" << endl;
    stringstream simulador_input("[(10, 100), (20, 200), (30, 300)]");
    simulador_input >> listOp;
    cout << "Lista luego de la lectura (operator<<): " << listOp << endl;

    cout << "\nOperador[]: " << endl;
    cout << "Indice [0] (operator[]): Dato -> " << listOp[0] << endl;
    cout << "Indice [2] (operator[]): Dato -> " << listOp[2] << endl;

    cout << "\nTest iteradores" << endl;
    cout << "Forward iterator: ";
    for (auto &v : listOp)
        cout << "(" << v << ") ";
    cout << endl;

    cout << "\nTest circular foreach (2 loops): " << endl;
    listOp.circularForEach(2, [](const auto &item){
        cout << "(" << item << ") ";
    });
    cout << endl;
}

void CircularDoubleLinkedListDemo(){
    //Completar
}


void ListsDemo(){
    cout << "\n===Pruebas de LinkedList===\n" << endl;
    LinkedListDemo();

    cout << "\n===Pruebas de DoubleLinkedList===\n" << endl;
    DoubleLinkedListDemo();

    cout << "\n===Pruebas de CircularLinkedList===\n" << endl;
    CircularLinkedListDemo();

    cout << "\n===Pruebas de CircularDoubleLinkedList===\n" << endl;
    CircularDoubleLinkedListDemo();

    cout << "\nFIN DE LAS PRUEBAS" << endl;
}