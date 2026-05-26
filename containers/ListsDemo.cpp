#include <iostream>
#include <fstream>
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
#include "circulardoublelinkedlist.h"
#include "traits.h"
#include "../types.h"

using namespace std;

// LinkedList Demo
template <typename Container>
void DemoLinkedList(Container& list, const string& name) {    
    cout << "\nDemo de " << name << endl;
    // Agregar múltiples elementos
    cout << "\n1. Insertando 5 elementos con push_back:" << endl;
    list.push_back(50, 10);
    list.push_back(30, 20);
    list.push_back(70, 30);
    list.push_back(20, 40);
    list.push_back(80, 50);
    cout << "   Tamaño: " << list.size() << endl;
    cout << "   Estado de la lista:" << list << endl;
    
    // Remover del frente
    cout << "2. Pop front :" << endl;
    auto [data, ref] = list.pop_front();
    cout << "   Removido: (" << data << ", " << ref << ")" << endl;
    cout << "   Tamaño: " << list.size() << endl;
    
    // Insertar elemento
    cout << "3. Insertando elemento 60 con insert():" << endl;
    list.insert(60, 60);
    cout << "   Tamaño: " << list.size() << endl;
    
    // Pop front nuevamente
    cout << "4. Pop front adicional:" << endl;
    auto [data2, ref2] = list.pop_front();
    cout << "   Removido: (" << data2 << ", " << ref2 << ")" << endl;
    cout << "   Tamaño final: " << list.size() << endl;

    cout << "   Estado de la lista: [ ";
    for (const auto& item : list) {
        cout << item << " ";
    }
    cout << "]" << endl;
}

// DoubleLinkedList Demo
template <typename Container>
void DemoDoubleLinkedList(Container& dll, const string& name) {
    cout << "\nDemo de " << name << endl;
    // Agregar por atrás
    cout << "\n1. Insertando 3 elementos con push_back:" << endl;
    dll.push_back(50, 10);
    dll.push_back(30, 20);
    dll.push_back(70, 30);
    cout << "   Tamaño: " << dll.size() << endl;
    cout << "   Estado de la lista:" << dll << endl;
    
    // Remover por atrás
    cout << "2. Pop back (removiendo por el final):" << endl;
    auto [data1, ref1] = dll.pop_back();
    cout << "   Removido: (" << data1 << ", " << ref1 << ")" << endl;
    
    // Agregar por adelante
    cout << "3. Agregando elemento por push_front: (100, 100)" << endl;
    dll.push_front(100, 100);
    cout << "   Tamaño: " << dll.size() << endl;
    
    // Remover del frente
    cout << "4. Pop front:" << endl;
    auto [data2, ref2] = dll.pop_front();
    cout << "   Removido: (" << data2 << ", " << ref2 << ")" << endl;
    cout << "   Tamaño final: " << dll.size() << endl;
    
    // Insertar con orden
    cout << "5. Insertando elemento 40:" << endl;
    dll.insert(40, 40);
    cout << "   Tamaño: " << dll.size() << endl;
    cout << "   Estado de la lista: [ ";
    for (const auto& item : dll) {
        cout << item << " ";
    }
    cout << "]" << endl; 
}

// CircularLinkedList Demo
template <typename Container>
void DemoCircularLinkedList(Container& cll, const string& name) {
    cout << "\nDemo de " << name << endl;
    // Agregar elementos
    cout << "\n1. Agregando elementos (circular):" << endl;
    cll.push_back(10, 1);
    cll.push_back(20, 2);
    cll.push_back(30, 3);
    cll.push_back(40, 4);
    cout << "   Tamaño: " << cll.size() << endl;
    cout << "   Estado de la lista: " << cll << endl;
    Print(cll, cout);

    cout << "   (Estructura circular: último → primero)" << endl;
    
    // Agregar al frente
    cout << "2. Agregando al frente con push_front: (5, 5)" << endl;
    cll.push_front(5, 5);
    cout << "   Tamaño: " << cll.size() << endl;
    
    // Remover del frente
    cout << "3. Pop front:" << endl;
    auto [data, ref] = cll.pop_front();
    cout << "   Removido: (" << data << ", " << ref << ")" << endl;
    
    // Insertar con orden
    cout << "4. Insertando elemento 25:" << endl;
    cll.insert(25, 25);
    cout << "   Tamaño final: " << cll.size() << endl;

    cout << "   Dando 2 vueltas a la lista circular: ";
    cll.circularForEach(2, [](auto& cll) { cout << cll << " "; });
    cout << endl;
}

// CircularDoubleLinkedList Demo
template <typename Container>
void DemoCircularDoubleLinkedList(Container& cdll, const string& name) {
    cout << "\nDemo de " << name << endl;
    // Agregar elementos
    cout << "\n1. Agregando elementos con push_back:" << endl;
    cdll.push_back(15, 1);
    cdll.push_back(25, 2);
    cdll.push_back(35, 3);
    cout << "   Tamaño: " << cdll.size() << endl;
    cout << "   Estado de la lista: " << cdll << endl;
    Print(cdll, cout); 
    
    // Push front
    cout << "2. Agregando al frente con push_front: (5, 5)" << endl;
    cdll.push_front(5, 5);
    cout << "   Tamaño: " << cdll.size() << endl;
    
    // Pop back
    cout << "3. Pop back (removiendo del final):" << endl;
    auto [data1, ref1] = cdll.pop_back();
    cout << "   Removido: (" << data1 << ", " << ref1 << ")" << endl;
    
    // Pop front
    cout << "4. Pop front:" << endl;
    auto [data2, ref2] = cdll.pop_front();
    cout << "   Removido: (" << data2 << ", " << ref2 << ")" << endl;
    
    // Insert
    cout << "5. Insertando 30:" << endl;
    cdll.insert(30, 30);
    cout << "   Tamaño final: " << cdll.size() << endl;
    cout << "   Dando 2 vueltas a la lista circular: ";
    cdll.circularForEach(2, 1,[](auto& cdll) { cout << cdll << " "; });
    cout << endl;


}

void ListsDemo() {   
    cout << "\nLINKED LISTS" << endl;
    LinkedList<AscendingTrait<LLNode<T1>>> ll;
    DemoLinkedList(ll, "LinkedList Ascendente");
    LinkedList<DescendingTrait<LLNode<T1>>> ll_desc;
    DemoLinkedList(ll_desc, "LinkedList Descendente");

    cout << "\nDOUBLE LINKED LISTS" << endl;    
    DoubleLinkedList<AscendingTrait<DLLNode<T1>>> dll;
    DemoDoubleLinkedList(dll, "DoubleLinkedList Ascendente");
    DoubleLinkedList<DescendingTrait<DLLNode<T1>>> dll_desc;
    DemoDoubleLinkedList(dll_desc, "DoubleLinkedList Descendente");
    
    cout << "\nCIRCULAR LINKED LISTS" << endl;    
    CircularLinkedList<AscendingTrait<LLNode<T1>>> cll;
    DemoCircularLinkedList(cll, "CircularLinkedList Ascendente");
    CircularLinkedList<DescendingTrait<LLNode<T1>>> cll_desc;
    DemoCircularLinkedList(cll_desc, "CircularLinkedList Descendente");
    
    cout << "\nCIRCULAR DOUBLE LISTS" << endl;
    CircularDoubleLinkedList<AscendingTrait<DLLNode<T1>>> cdll;
    DemoCircularDoubleLinkedList(cdll, "CircularDoubleLinkedList Ascendente");
    CircularDoubleLinkedList<DescendingTrait<DLLNode<T1>>> cdll_desc;
    DemoCircularDoubleLinkedList(cdll_desc, "CircularDoubleLinkedList Descendente");
}
