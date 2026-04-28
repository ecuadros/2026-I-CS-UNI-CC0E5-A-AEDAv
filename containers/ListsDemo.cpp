#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include "doublelinkedlist.h"
#include "../types.h"
#include "linkedlist.h"
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
    cout<<"Escritura"<<endl;
    cout << list << endl;
    // Grabar la lista en un archivo
    ofstream os(fileName);
    os << list << endl;
    os.close();

    Container listFromFile;
    // Leer la lista desde un archivo
    ifstream is(fileName);
    cout<<"Lectura"<<endl;
    is >> listFromFile;
    cout << listFromFile << endl;
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



    //push front / push back
    DoubleLinkedList<AscendingDLLTrait<T1>> pushList;
    cout << "\n3. PUSH FRONT / PUSH BACK" << endl;
    pushList.push_back(20,2); pushList.push_back(25,25);
    pushList.push_front(100,1); pushList.push_front(17,5);
    cout << "Luego de usar push: " << pushList << endl;

    //pop front / pop back 
    cout << "\n2. POP FRONT / POP BACK" << endl;
    auto [d1,r1] = pushList.pop_front();
    cout << "  pop_front -> (" << d1 << "," << r1 << ") | lista: " << pushList << endl;
    auto [d2,r2] = pushList.pop_back();
    cout << "  pop_back  -> (" << d2 << "," << r2 << ") | lista: " << pushList << endl;



    //iterador forward
    cout << "\n4. ITERADOR FORWARD" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> iterList;
    iterList.insert(20,2); iterList.insert(25,25); iterList.insert(100,1);
    iterList.insert(17,5); iterList.insert(5,50);
    cout << "  fwd: ";
    for (auto &v : iterList) cout << v << " ";
    cout << endl;

    //iterador backward
    cout << "\n5. ITERADOR BACKWARD" << endl;
    cout << "  bwd: ";
    for (auto it = iterList.rbegin(); it != iterList.rend(); ++it)
        cout << *it << " ";
    cout << endl;

    //foreach yy reverseforeach
    cout << "\nFOREACH / REVERSEFOREACH" << endl;
    cout << "  ForEach fwd:        ";
    iterList.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;
    cout << "  ReverseForEach bwd: ";
    iterList.ReverseForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    //operator[]
    cout << "\n7. OPERATOR[]" << endl;
    cout << "  [0]=" << iterList[0] << " [2]=" << iterList[2]
         << " [4]=" << iterList[4] << endl;

    //copy constructor
    cout << "\n8. COPY CONSTRUCTOR" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> copied(iterList);
    cout << "  Original: " << iterList << endl;
    cout << "  Copiada:  " << copied   << endl;

    //move constructor
    cout << "\n9. MOVE CONSTRUCTOR" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> moved(std::move(copied));
    cout << "  Moved: " << moved << endl;
    cout << "  Original tras move (size=0): " << copied.size() << endl;

    //operator>>
    cout << "\n11. OPERATOR>>" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> streamList;
    stringstream ss("[(17, 5), (15, 2), (30, 12)]");
    ss >> streamList;
    cout << "  Stream desordenado -> lista: " << streamList << endl;

    //concurrencia
    cout << "\n12. CONCURRENCIA (5 hilos x 1000 push_front)" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado 5000: " << concList.size() << endl;

}

void CircularLinkedListDemo(){
    CircularLinkedList<AscendingCLLTrait<T1>> list3;
    DemoList(list3, "AscCLL.txt");

    CircularLinkedList<DescendingCLLTrait<T1>> list4;
    DemoList(list4, "DescCLL.txt");



    //push front ,  push back , pop front,  pop_back
    cout << "\n2. PUSH FRONT / PUSH BACK / POP FRONT / POP BACK" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> pushList;
    pushList.push_back(17,25);pushList.push_back(21,17);pushList.push_front(100,2);pushList.push_front(25,0);
    cout << "  Despues de push_back: " << pushList << endl;
    auto [d1,r1] = pushList.pop_front();
    cout << "  pop_front -> (" << d1 << "," << r1 << ") | lista: " << pushList << endl;
    auto [d2,r2] = pushList.pop_back();
    cout << "  pop_back  -> (" << d2 << "," << r2 << ") | lista: " << pushList << endl;

    //naturaleza circular y circularForEach N vueltas
    cout << "\n3. NATURALEZA CIRCULAR circularForEach x2 vueltas" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> circList;
    circList.insert(2,15);circList.insert(4,18);circList.insert(31,3);
    cout << "  Lista: " << circList << endl;
    cout << "  x2 vueltas: ";
    circList.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;

    //iterador forward
    cout << "\n4. ITERADOR FORWARD" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> iterList;
    iterList.insert(2,15); iterList.insert(4,18); iterList.insert(31,3);
    iterList.insert(4,40); iterList.insert(5,50);
    cout << "  ranged-for (1 vuelta exacta): ";
    for (auto &v : iterList) cout << v << " ";
    cout << endl;

    //forEach
    cout << "\n5. FOREACH" << endl;
    cout << "  ForEach: ";
    iterList.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    //operator[]
    cout << "\n6. OPERATOR[]" << endl;
    cout << "  [0]=" << iterList[0] << " [2]=" << iterList[2]
         << " [4]=" << iterList[4] << endl;

    //copy constructor y move constructor
    cout << "\n7. COPY / MOVE CONSTRUCTORS" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> orig;
    orig.insert(10,1); orig.insert(2,15); orig.insert(17,2);
    CircularLinkedList<AscendingCLLTrait<T1>> copied(orig);
    cout << "  Orig:   " << orig   << endl;
    cout << "  Copied: " << copied << endl;
    cout << "  Copied x2 vueltas: ";
    copied.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> moved(std::move(orig));
    cout << "  Moved:  " << moved << endl;
    cout << "  Orig tras move (size=0): " << orig.size() << endl;


    //operator>>
    cout << "\n9. OPERATOR>> DESDE STREAM (respeta Comp)" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> streamList;
    stringstream ss("[(2, 15), (4, 18), (31, 3)]");
    ss >> streamList;
    cout << "  Stream desordenado -> lista: " << streamList << endl;

    //concurrencia
    cout << "\n10. CONCURRENCIA (5 hilos x 1000 push_front)" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado 5000: " << concList.size() << endl;

}

void CircularDoubleLinkedListDemo(){
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> list5;
    DemoList(list5, "AscCDLL.txt");

    CircularDoubleLinkedList<DescendingCDLLTrait<T1>> list6;
    DemoList(list6, "DescCDLL.txt");

    //push front , push back,pop front, pop back
    cout << "\n2. PUSH FRONT / PUSH BACK / POP FRONT / POP BACK" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> pushList;
    pushList.push_back(35,3); pushList.push_back(27,9);
    pushList.push_front(17,12); pushList.push_front(50,47);
    cout << "  Despues de pushes: " << pushList << endl;
    auto [d1,r1] = pushList.pop_front();
    cout << "  pop_front -> (" << d1 << "," << r1 << ") | lista: " << pushList << endl;
    auto [d2,r2] = pushList.pop_back();
    cout << "  pop_back  -> (" << d2 << "," << r2 << ") | lista: " << pushList << endl;

    //fwd y bwd en N vueltas
    cout << "\n3. NATURALEZA CIRCULAR DOBLE (fwd y bwd x2 vueltas)" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> circList;
    circList.insert(35,3); circList.insert(27,9); circList.insert(17,12);
    cout << "  Lista: " << circList << endl;
    cout << "  fwd x2: ";
    circList.circularForEach(2,  1, [](T1 &v){ cout << v << " "; });
    cout << endl;
    cout << "  bwd x2: ";
    circList.circularForEach(2, -1, [](T1 &v){ cout << v << " "; });
    cout << endl;

    //iteradores forward y backward
    cout << "\n4. ITERADORES FORWARD Y BACKWARD (ranged-for)" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> iterList;
    iterList.insert(1,10); iterList.insert(2,20); iterList.insert(3,30);
    iterList.insert(4,40); iterList.insert(5,50);
    cout << "  ranged-for fwd: ";
    for (auto &v : iterList) cout << v << " ";
    cout << endl;
    cout << "  ranged-for bwd: ";
    for (auto it = iterList.rbegin(); it != iterList.rend(); ++it)
        cout << *it << " ";
    cout << endl;

    //foreach y reverseforeach
    cout << "\n5. FOREACH / REVERSEFOREACH CON LOCK" << endl;
    cout << "  ForEach fwd:        ";
    iterList.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;
    cout << "  ReverseForEach bwd: ";
    iterList.ReverseForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    //operator[]
    cout << "\n6. OPERATOR[]" << endl;
    cout << "  [0]=" << iterList[0] << " [2]=" << iterList[2]
         << " [4]=" << iterList[4] << endl;

    //copy constructor move constructor
    cout << "\n7. COPY / MOVE CONSTRUCTORS" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> orig;
    orig.insert(10,1); orig.insert(20,2); orig.insert(30,3);
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> copied(orig);
    cout << "  Orig:   " << orig   << endl;
    cout << "  Copied: " << copied << endl;
    cout << "  Copied fwd x2: ";
    copied.circularForEach(2,  1, [](T1 &v){ cout << v << " "; });
    cout << endl;
    cout << "  Copied bwd x2: ";
    copied.circularForEach(2, -1, [](T1 &v){ cout << v << " "; });
    cout << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> moved(std::move(orig));
    cout << "  Moved:  " << moved << endl;
    cout << "  Orig tras move (size=0): " << orig.size() << endl;


    //operator>>
    cout << "\n8. OPERATOR>> DESDE STREAM" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> streamList;
    stringstream ss("[(30, 3), (10, 1), (20, 2)]");
    ss >> streamList;
    cout << "  Stream desordenado -> lista: " << streamList << endl;

    //Concurrencia
    cout << "\n9 CONCURRENCIA" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado 5000: " << concList.size() << endl;
}
    

void ListsDemo(){
    std::cout<<"Pruebas LInked list"<<endl;
    LinkedListDemo();
    std::cout<<"Pruebas CircularLinkedList"<<endl;
    CircularLinkedListDemo();
    std::cout<<"Pruebas DoubleLinkedList"<<endl;
    DoubleLinkedListDemo();
    std::cout<<"Pruebas CircularDoubleLinkedList"<<endl;
    CircularDoubleLinkedListDemo();
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

void TestBasicos(){
    
}
// void ListsDemo(){
//     TestBasicos();
//     TestConcurrencia();
//     TestOperators();
//     cout << "\n=== FIN DE LAS PRUEBAS ===" << endl;
// }