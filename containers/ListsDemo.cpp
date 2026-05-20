#include <iostream>
#include <fstream>
#include <string>
#include<sstream>
#include<thread>
#include<vector>

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

    // Grabar la lista en un archivo
    ofstream os(fileName);
    os << list << endl;
    
    // Leer la lista desde un archivo
    ifstream is(fileName);
    is >> list;
    cout << list << endl;
}

void LinkedListDemo(){
    LinkedList<DescendingLinkedListTrait<T1>> list;
    list.insert(2,200);
    list.insert(1,100);
    list.insert(3,300);
    LinkedList<DescendingLinkedListTrait<T1>> listMove(std::move(list));
    auto [data_front, ref_front] = listMove.pop_front();
    cout << "Pop -> Dato: " << data_front << " | Metadato (Ref): " << ref_front << endl;
    LinkedList<AscendingLinkedListTrait<T1>> listconc;
    auto worker = [&listconc](int thread_id) {
        for(int i = 0; i < 1000; i++) {
            listconc.push_front(i, thread_id);
        }
    };

    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);

    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "Tamano de la lista (Esperado 5000): " << listconc.size() << endl;
    if(listconc.size() == 5000) {
        cout << "ESTADO: EXITO - El shared_mutex previno condiciones de carrera perfectamente." << endl;
    } else {
        cout << "ESTADO: FALLO - Hubo corrupcion de memoria." << endl;
    }

    cout << "\nESCRITURA/LECTURA ARCHIVOS" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list1;
    DemoList(list1, "AscLL.txt");
    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");

    LinkedList<AscendingLinkedListTrait<T1>> listOp;
    cout << "Simulando lectura: [(10, 100), (20, 200), (30, 300)]" << endl;

    stringstream simulador_input("[(10, 100), (20, 200), (30, 300)]");
    simulador_input >> listOp;
    cout << "Lista luego de la lectura (operator<<): " << listOp << endl;
    cout << "Accediendo al indice [0] (operator[]): Dato -> " << listOp[0] << endl;
    cout << "Accediendo al indice [2] (operator[]): Dato -> " << listOp[2] << endl;


}

void DoubleLinkedListDemo() {
    cout << "------------- DEMO DoubleLinkedList -------------" << endl;

    // 1. Insercion ascendente
    DoubleLinkedList<AscendingDLLTrait<T1>> listAsc;
    listAsc.push_back(10, 1);
    listAsc.push_back(20, 2);
    listAsc.push_back(30, 3);
    listAsc.push_back(5,  4);
    cout << "Ascendente (push_back): " << listAsc << endl;

    // 2. Forward con ForEach
    cout << "ForEach (suma +1): ";
    listAsc.ForEach([](T1& x){ x += 1; });
    cout << listAsc << endl;

    // 3. Insercion descendente
    DoubleLinkedList<DescendingDLLTrait<T1>> listDesc;
    listDesc.push_front(100, 10);
    listDesc.push_front(200, 20);
    listDesc.push_front(50,  30);
    cout << "Descendente (push_front): " << listDesc << endl;

    // 4. operator<< / operator>> para un archivo
    ofstream of("DLL.txt");
    of << listAsc << endl;
    of.close();
    DoubleLinkedList<AscendingDLLTrait<T1>> listLoaded;
    ifstream is("DLL.txt");
    is >> listLoaded;
    cout << "Leída desde archivo: " << listLoaded << endl;

    //5. Bucle nativo range-for
    cout << "Range-for: ";
    for (auto& val : listAsc)
        cout << val << " ";

    // 7. Copy constructor
    DoubleLinkedList<AscendingDLLTrait<T1>> listCopy(listAsc);
    cout << "Copia: " << listCopy << endl;

    // 8. Move constructor
    DoubleLinkedList<AscendingDLLTrait<T1>> listMoved(std::move(listCopy));
    cout << "Movida: " << listMoved << endl;
    cout << "Original tras move (debe estar vacía): " << listCopy << endl;

    // 9. Concurrencia
    DoubleLinkedList<AscendingDLLTrait<T1>> listConc;
    auto worker = [&listConc](int id){
        for (int i = 0; i < 1000; i++)
            listConc.push_back(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3),t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "Concurrencia (esperado 5000): " << listConc.size() << endl;
}

void CircularLinkedListDemo() {
    cout << "\n------------- DEMO CircularLinkedList -------------" << endl;
 
    // 1. push_back y push_front
    CircularLinkedList<AscendingCLLTrait<T1>> listAsc;
    listAsc.push_back(10, 1);
    listAsc.push_back(20, 2);
    listAsc.push_back(30, 3);
    cout << "push_back(10,20,30): " << listAsc << endl;
    listAsc.push_front(5, 0);
    cout << "push_front(5):       " << listAsc << endl;
 
    // 2. insert ordenado asc y desc
    CircularLinkedList<AscendingCLLTrait<T1>> listIns;
    listIns.insert(28, 15); listIns.insert(17, 25);
    listIns.insert(8,  35); listIns.insert(4,  45); listIns.insert(35, 55);
    cout << "insert ordenado asc: " << listIns << endl;
 
    CircularLinkedList<DescendingCLLTrait<T1>> listDesc;
    listDesc.insert(28, 15); listDesc.insert(17, 25);
    listDesc.insert(8,  35); listDesc.insert(4,  45); listDesc.insert(35, 55);
    cout << "insert ordenado desc:" << listDesc << endl;
 
    // 3. ForEach
    listAsc.ForEach([](T1& x){ x += 1; });
    cout << "ForEach (+1):        " << listAsc << endl;
 
    // 4. operator<< / >>
    ofstream of("CLL.txt");
    of << listIns << endl;
    of.close();
    CircularLinkedList<AscendingCLLTrait<T1>> listLoaded;
    ifstream is("CLL.txt");
    is >> listLoaded;
    cout << "Leida desde archivo: " << listLoaded << endl;
 
    // 5. Copy y Move constructor
    CircularLinkedList<AscendingCLLTrait<T1>> listCopy(listIns);
    cout << "Copia:               " << listCopy << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> listMoved(std::move(listCopy));
    cout << "Movida:              " << listMoved << endl;
    cout << "Original tras move:  " << listCopy  << endl;
 
    // 6. Concurrencia
    CircularLinkedList<AscendingCLLTrait<T1>> listConc;
    auto worker = [&listConc](int id){
        for (int i = 0; i < 1000; i++) listConc.push_back(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "Concurrencia (esperado 5000): " << listConc.size() << endl;
    cout << (listConc.size() == 5000 ? "ESTADO: EXITO" : "ESTADO: FALLO") << endl;
}
 
void CircularDoubleLinkedListDemo() {
    cout << "\n------------- DEMO CircularDoubleLinkedList -------------" << endl;
 
    // 1. push_back y push_front
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> listAsc;
    listAsc.push_back(10, 1);
    listAsc.push_back(20, 2);
    listAsc.push_back(30, 3);
    cout << "push_back(10,20,30): " << listAsc << endl;
    listAsc.push_front(5, 0);
    cout << "push_front(5):       " << listAsc << endl;
 
    // 2. insert ordenado asc y desc
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> listIns;
    listIns.insert(28, 15); listIns.insert(17, 25);
    listIns.insert(8,  35); listIns.insert(4,  45); listIns.insert(35, 55);
    cout << "insert ordenado asc: " << listIns << endl;
 
    CircularDoubleLinkedList<DescendingCDLLTrait<T1>> listDesc;
    listDesc.insert(28, 15); listDesc.insert(17, 25);
    listDesc.insert(8,  35); listDesc.insert(4,  45); listDesc.insert(35, 55);
    cout << "insert ordenado desc:" << listDesc << endl;
 
    // 3. ForEach forward
    listAsc.ForEach([](T1& x){ x += 1; });
    cout << "ForEach (+1):        " << listAsc << endl;
 
    // 4. ReverseForEach
    cout << "ReverseForEach:      ";
    listIns.ReverseForEach([](T1& x){ cout << x << " "; });
    cout << endl;
 
    // 5. operator<< / >>
    ofstream of("CDLL.txt");
    of << listIns << endl;
    of.close();
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> listLoaded;
    ifstream is("CDLL.txt");
    is >> listLoaded;
    cout << "Leida desde archivo: " << listLoaded << endl;
 
    // 6. Copy y Move constructor
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> listCopy(listIns);
    cout << "Copia:               " << listCopy << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> listMoved(std::move(listCopy));
    cout << "Movida:              " << listMoved << endl;
    cout << "Original tras move:  " << listCopy  << endl;
 
    // 7. Concurrencia
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> listConc;
    auto worker = [&listConc](int id){
        for (int i = 0; i < 1000; i++) listConc.push_back(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "Concurrencia (esperado 5000): " << listConc.size() << endl;
    cout << (listConc.size() == 5000 ? "ESTADO: EXITO" : "ESTADO: FALLO") << endl;
}

void TestConcurrencia() {
    cout << "\nTEST DE CONCURRENCIA" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;

    // 5 hilos van a intentar meter 1000 elementos cada uno al mismo tiempo
    auto worker = [&list](int thread_id) {
        for(int i = 0; i < 1000; i++) {
            list.push_front(i, thread_id);
        }
    };

    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);

    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();

    cout << "Se lanzaron 5 hilos insertando 1000 elementos simultaneamente." << endl;
    cout << "Tamano de la lista (Esperado 5000): " << list.size() << endl;
    if(list.size() == 5000) {
        cout << "ESTADO: EXITO - El shared_mutex previno condiciones de carrera perfectamente." << endl;
    } else {
        cout << "ESTADO: FALLO - Hubo corrupcion de memoria." << endl;
    }
}

void TestOperators() {
    cout << "\nTEST DE OPERADORES" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;
    
    // 1. Probamos operator>> (Lectura)
    cout << "Simulando lectura desde formato: [(10, 100), (20, 200), (30, 300)]" << endl;
    stringstream simulador_input("[(10, 100), (20, 200), (30, 300)]");
    simulador_input >> list;

    // 2. Probamos operator<< (Escritura)
    cout << "Lista luego de la lectura (operator<<): " << list << endl;
    
    // 3. Probamos operator[] (Acceso seguro por indice)
    cout << "Accediendo al indice [0] (operator[]): Dato -> " << list[0] << endl;
    cout << "Accediendo al indice [2] (operator[]): Dato -> " << list[2] << endl;
    
    // Probamos la excepcion del operator[] (Descomentar para probar)
    // cout << "Probando fuera de rango: " << list[5] << endl; // Lanzara la excepcion
}


void ListsDemo(){
    //TestBasicos();
    //TestConcurrencia();
    //TestOperators();
    DoubleLinkedListDemo();
    CircularLinkedListDemo();
    CircularDoubleLinkedListDemo();
}