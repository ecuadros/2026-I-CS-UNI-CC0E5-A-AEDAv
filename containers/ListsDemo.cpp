#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <iomanip>

#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularLinkedList.h"
#include "circularDoubleLinkedList.h"

using namespace std;

// Prueba genérica: Funciona para cualquier contenedor que tenga insert, operator<< y operator>>
template <typename Container>
void DemoList(Container& list, string fileName){
    cout << "  -> Insertando valores de prueba (28, 17, 8, 4, 35)..." << endl;
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8,35);
    list.insert(4,45);
    list.insert(35,55);
    cout << "  [RESULTADO] Original:      " << list << endl;
    
    cout << "  -> Guardando en archivo: " << fileName << "..." << endl;
    ofstream os(fileName);
    os << list << endl;
    os.close();
    
    cout << "  -> Leyendo desde archivo para verificar integridad..." << endl;
    Container listFromFile;
    ifstream  is(fileName);
    is >> listFromFile;
    cout << "  [RESULTADO] Leida archivo: " << listFromFile << endl;
}

// Demo de LinkedList (Simple)
void LinkedListDemo()
{
    cout << "==========================================================" << endl;
    cout << "                DEMO: LINKED LIST (SIMPLE)                " << endl;
    cout << "==========================================================" << endl;

    cout << "\n1. PRUEBA DE MOVE CONSTRUCTOR (Big Five)" << endl;
    LinkedList<DescendingLinkedListTrait<T1>> list;
    list.insert(2, 200); list.insert(1, 100); list.insert(3, 300);
    cout << "  Lista original antes del move: " << list << endl;
    LinkedList<DescendingLinkedListTrait<T1>> listMove(std::move(list));
    cout << "  Lista nueva (movida):          " << listMove << endl;
    auto [data_front, ref_front] = listMove.pop_front();
    cout << "  [POP FRONT] Dato: " << data_front << " | Ref: " << ref_front << endl;

    cout << "\n2. PRUEBA DE CONCURRENCIA (Proteccion con shared_mutex)" << endl;
    cout << "  Ejecutando 5 hilos con 1000 push_front cada uno..." << endl;
    LinkedList<AscendingLinkedListTrait<T1>> listconc;
    auto worker = [&listconc](int id) {
        for (int i = 0; i < 1000; i++) listconc.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado: 5000 | Tamano actual: " << listconc.size() << endl;
    cout << "  ESTADO FINAL: " << (listconc.size()==5000 ? "[EXITO]" : "[FALLO]") << endl;

    cout << "\n3. PRUEBA DE ARCHIVOS (Escritura y Lectura con Traits)" << endl;
    cout << "  -- Caso Ascendente --" << endl;
    LinkedList<AscendingLinkedListTrait<T1>>  list1;
    DemoList(list1, "AscLL.txt");
    cout << "  -- Caso Descendente --" << endl;
    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");

    cout << "\n4. PRUEBA DE OPERADORES (Stream y Acceso Aleatorio)" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> listOp;
    stringstream ss("[(10, 100), (20, 200), (30, 300)]");
    cout << "  Parsing desde stringstream: [(10, 100), (20, 200), (30, 300)]" << endl;
    ss >> listOp;
    cout << "  [operator>>]: " << listOp << endl;
    cout << "  [operator[]]: list[0]=" << listOp[0] << ", list[2]=" << listOp[2] << endl;
}

// Demo de DoubleLinkedList
void DoubleLinkedListDemo(){
    cout << "==========================================================" << endl;
    cout << "               DEMO: DOUBLE LINKED LIST                   " << endl;
    cout << "==========================================================" << endl;

    cout << "\n1. INSERCION ORDENADA (Ascendente y Descendente)" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50); asc.insert(2,20); asc.insert(4,40);
    cout << "  Asc:  " << asc  << endl;

    DoubleLinkedList<DescendingDLLTrait<T1>> desc;
    desc.insert(3,30); desc.insert(1,10); desc.insert(5,50); desc.insert(2,20); desc.insert(4,40);
    cout << "  Desc: " << desc << endl;

    cout << "\n2. PUSH FRONT / PUSH BACK" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> pushList;
    pushList.push_back(20,2); pushList.push_back(30,3);
    pushList.push_front(10,1); pushList.push_front(5,0);
    cout << "  Lista resultante: " << pushList << endl;

    cout << "\n3. POP FRONT / POP BACK" << endl;
    auto [d1,r1] = pushList.pop_front();
    cout << "  pop_front -> (" << d1 << "," << r1 << ") | Lista: " << pushList << endl;
    auto [d2,r2] = pushList.pop_back();
    cout << "  pop_back  -> (" << d2 << "," << r2 << ") | Lista: " << pushList << endl;

    cout << "\n4. ITERADORES (Forward y Backward)" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> iterList;
    for(int i=1; i<=5; ++i) iterList.insert(i, i*10);
    cout << "  Forward Iterator:  ";
    for (auto &v : iterList) cout << v << " ";
    cout << "\n  Backward Iterator: ";
    for (auto it = iterList.rbegin(); it != iterList.rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "\n5. METODOS FUNCIONALES (ForEach y ReverseForEach)" << endl;
    cout << "  ForEach (fwd):        ";
    iterList.ForEach([](T1 &v){ cout << v << " "; });
    cout << "\n  ReverseForEach (bwd): ";
    iterList.ReverseForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    cout << "\n6. ACCESO POR INDICE (Optimizado O(n/2))" << endl;
    cout << "  Indice [0]: " << iterList[0] << " | [2]: " << iterList[2] << " | [4]: " << iterList[4] << endl;

    cout << "\n7. BIG FIVE: COPY Y MOVE CONSTRUCTORS" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> copied(iterList);
    cout << "  Copia (Copy Ctor): " << copied << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> moved(std::move(copied));
    cout << "  Movida (Move Ctor): " << moved << endl;
    cout << "  Estado origen tras move (size): " << copied.size() << endl;

    cout << "\n8. CONCURRENCIA" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano final (esperado 5000): " << concList.size() << endl;
}

// Demo de CircularLinkedList
void CircularLinkedListDemo(){
    cout << "==========================================================" << endl;
    cout << "               DEMO: CIRCULAR LINKED LIST                 " << endl;
    cout << "==========================================================" << endl;

    cout << "\n1. INSERCION ORDENADA" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50); asc.insert(2,20); asc.insert(4,40);
    cout << "  Ascendente:  " << asc  << endl;

    cout << "\n2. NATURALEZA CIRCULAR (Prueba de vueltas infinitas controladas)" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> circList;
    circList.insert(1,10); circList.insert(2,20); circList.insert(3,30);
    cout << "  Lista base: " << circList << endl;
    cout << "  Recorrido de 2 vueltas completas (circularForEach): ";
    circList.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;

    cout << "\n3. ITERADOR FORWARD (Termina tras una vuelta completa)" << endl;
    cout << "  ranged-for: ";
    for (auto &v : circList) cout << v << " ";
    cout << endl;

    cout << "\n4. BIG FIVE: COPY / MOVE CONSTRUCTORS" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> copied(circList);
    cout << "  Original: " << circList << endl;
    cout << "  Copia:    " << copied << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> moved(std::move(copied));
    cout << "  Movida:   " << moved << " | Origen tras move (size): " << copied.size() << endl;

    cout << "\n5. CONCURRENCIA" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano final (esperado 5000): " << concList.size() << endl;
}

// Demo de CircularDoubleLinkedList
void CircularDoubleLinkedListDemo(){
    cout << "==========================================================" << endl;
    cout << "            DEMO: CIRCULAR DOUBLE LINKED LIST             " << endl;
    cout << "==========================================================" << endl;

    cout << "\n1. INSERCION ORDENADA" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50); asc.insert(2,20); asc.insert(4,40);
    cout << "  Ascendente: " << asc  << endl;

    cout << "\n2. NATURALEZA CIRCULAR DOBLE (Fwd y Bwd con vueltas)" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> circList;
    circList.insert(1,10); circList.insert(2,20); circList.insert(3,30);
    cout << "  Lista base: " << circList << endl;
    cout << "  Fwd (2 vueltas): "; circList.circularForEach(2,  1, [](T1 &v){ cout << v << " "; });
    cout << "\n  Bwd (2 vueltas): "; circList.circularForEach(2, -1, [](T1 &v){ cout << v << " "; });
    cout << endl;

    cout << "\n3. ITERADORES (Fwd y Bwd)" << endl;
    cout << "  Ranged-for Forward:  "; for (auto &v : circList) cout << v << " ";
    cout << "\n  Ranged-for Backward: "; for (auto it = circList.rbegin(); it != circList.rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "\n4. BIG FIVE: COPY / MOVE CONSTRUCTORS" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> copied(circList);
    cout << "  Copia (Copy Ctor): " << copied << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> moved(std::move(copied));
    cout << "  Movida (Move Ctor): " << moved << endl;

    cout << "\n5. CONCURRENCIA" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano final (esperado 5000): " << concList.size() << endl;
}

void ListsDemo()
{
    LinkedListDemo();
    cout << "\n" << string(60, '-') << "\n" << endl;
    DoubleLinkedListDemo();
    cout << "\n" << string(60, '-') << "\n" << endl;
    CircularLinkedListDemo();
    cout << "\n" << string(60, '-') << "\n" << endl;
    CircularDoubleLinkedListDemo();

    cout << "\n==========================================================" << endl;
    cout << "                FIN DE TODAS LAS PRUEBAS                  " << endl;
    cout << "==========================================================" << endl;
}
