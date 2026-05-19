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

template <typename Container>
void DemoList(Container& list, string fileName){
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8, 35);
    list.insert(4, 45);
    list.insert(35, 55);
    cout << "  original : " << list << endl;

    ofstream os(fileName);
    os << list << endl;
    os.close();

    Container listFromFile;
    ifstream  is(fileName);
    is >> listFromFile;
    cout << "  releida  : " << listFromFile << endl;
}

void LinkedListDemo()
{
    cout << "\n--- LinkedList ---" << endl;

    cout << "\nmove constructor" << endl;
    LinkedList<DescendingLinkedListTrait<T1>> list;
    list.insert(2, 200); list.insert(1, 100); list.insert(3, 300);
    cout << "  antes  : " << list << endl;
    LinkedList<DescendingLinkedListTrait<T1>> listMove(std::move(list));
    cout << "  movida : " << listMove << endl;
    auto [data_front, ref_front] = listMove.pop_front();
    cout << "  pop_front: " << data_front << " (ref=" << ref_front << ")" << endl;

    cout << "\nconcurrencia (5 hilos x 1000)" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> listconc;
    auto worker = [&listconc](int id) {
        for (int i = 0; i < 1000; i++) listconc.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  size = " << listconc.size() << endl;

    cout << "\npersistencia (asc y desc)" << endl;
    LinkedList<AscendingLinkedListTrait<T1>>  list1;
    DemoList(list1, "AscLL.txt");
    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");

    cout << "\nparseo desde stringstream" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> listOp;
    stringstream ss("[(10, 100), (20, 200), (30, 300)]");
    ss >> listOp;
    cout << "  " << listOp << endl;
    cout << "  list[0]=" << listOp[0] << ", list[2]=" << listOp[2] << endl;
}

void DoubleLinkedListDemo(){
    cout << "\n--- DoubleLinkedList ---" << endl;

    cout << "\ninsercion ordenada" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50); asc.insert(2,20); asc.insert(4,40);
    cout << "  asc  : " << asc  << endl;

    DoubleLinkedList<DescendingDLLTrait<T1>> desc;
    desc.insert(3,30); desc.insert(1,10); desc.insert(5,50); desc.insert(2,20); desc.insert(4,40);
    cout << "  desc : " << desc << endl;

    cout << "\npush / pop" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> pushList;
    pushList.push_back(20,2); pushList.push_back(30,3);
    pushList.push_front(10,1); pushList.push_front(5,0);
    cout << "  push : " << pushList << endl;

    auto [d1,r1] = pushList.pop_front();
    cout << "  pop_front -> " << d1 << "," << r1 << " | " << pushList << endl;
    auto [d2,r2] = pushList.pop_back();
    cout << "  pop_back  -> " << d2 << "," << r2 << " | " << pushList << endl;

    cout << "\niteradores fwd / bwd" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> iterList;
    for(int i=1; i<=5; ++i) iterList.insert(i, i*10);
    cout << "  fwd: ";
    for (auto &v : iterList) cout << v << " ";
    cout << "\n  bwd: ";
    for (auto it = iterList.rbegin(); it != iterList.rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "\nForEach / ReverseForEach" << endl;
    cout << "  fwd: ";
    iterList.ForEach([](T1 &v){ cout << v << " "; });
    cout << "\n  bwd: ";
    iterList.ReverseForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    cout << "\noperator[] : " << iterList[0] << " " << iterList[2] << " " << iterList[4] << endl;

    cout << "\ncopy / move" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> copied(iterList);
    cout << "  copy : " << copied << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> moved(std::move(copied));
    cout << "  move : " << moved << " (origen size=" << copied.size() << ")" << endl;

    cout << "\nconcurrencia" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  size = " << concList.size() << endl;

    cout << "\npersistencia (asc y desc)" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>>  dllAsc;
    DemoList(dllAsc,  "AscDLL.txt");
    DoubleLinkedList<DescendingDLLTrait<T1>> dllDesc;
    DemoList(dllDesc, "DescDLL.txt");

    cout << "\nLinkedList* -> DoubleLinkedList" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> derived;
    LinkedList<AscendingDLLTrait<T1>>* pBase = &derived;
    pBase->push_front(10, 100);
    pBase->push_back (30, 300);
    pBase->insert    (20, 200);
    cout << "  " << *pBase << endl;
    cout << "  bwd: ";
    for (auto it = derived.rbegin(); it != derived.rend(); ++it) cout << *it << " ";
    cout << endl;
}

void CircularLinkedListDemo(){
    cout << "\n--- CircularLinkedList ---" << endl;

    cout << "\ninsercion ordenada" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50); asc.insert(2,20); asc.insert(4,40);
    cout << "  " << asc  << endl;

    cout << "\ncircularForEach (2 vueltas)" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> circList;
    circList.insert(1,10); circList.insert(2,20); circList.insert(3,30);
    cout << "  base : " << circList << endl;
    cout << "  loop : ";
    circList.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;

    cout << "\nranged-for (corta tras una vuelta)" << endl;
    cout << "  ";
    for (auto &v : circList) cout << v << " ";
    cout << endl;

    cout << "\ncopy / move" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> copied(circList);
    cout << "  orig : " << circList << endl;
    cout << "  copy : " << copied << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> moved(std::move(copied));
    cout << "  move : " << moved << " (origen size=" << copied.size() << ")" << endl;

    cout << "\nconcurrencia" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  size = " << concList.size() << endl;

    cout << "\npersistencia (asc y desc)" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>>  cllAsc;
    DemoList(cllAsc,  "AscCLL.txt");
    CircularLinkedList<DescendingLinkedListTrait<T1>> cllDesc;
    DemoList(cllDesc, "DescCLL.txt");

    cout << "\nLinkedList* -> CircularLinkedList" << endl;
    CircularLinkedList<AscendingLinkedListTrait<T1>> derived;
    LinkedList<AscendingLinkedListTrait<T1>>* pBase = &derived;
    pBase->push_back(1, 10);
    pBase->push_back(2, 20);
    pBase->push_back(3, 30);
    cout << "  size = " << pBase->size() << endl;
    cout << "  vista (via derivada): " << derived << endl;
    cout << "  loop : ";
    derived.circularForEach(2, [](T1& v){ cout << v << " "; });
    cout << endl;
}

void CircularDoubleLinkedListDemo(){
    cout << "\n--- CircularDoubleLinkedList ---" << endl;

    cout << "\ninsercion ordenada" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50); asc.insert(2,20); asc.insert(4,40);
    cout << "  " << asc << endl;

    cout << "\ncircularForEach fwd / bwd (2 vueltas)" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> circList;
    circList.insert(1,10); circList.insert(2,20); circList.insert(3,30);
    cout << "  base : " << circList << endl;
    cout << "  fwd  : "; circList.circularForEach(2,  1, [](T1 &v){ cout << v << " "; });
    cout << "\n  bwd  : "; circList.circularForEach(2, -1, [](T1 &v){ cout << v << " "; });
    cout << endl;

    cout << "\nranged-for fwd / bwd" << endl;
    cout << "  fwd : "; for (auto &v : circList) cout << v << " ";
    cout << "\n  bwd : "; for (auto it = circList.rbegin(); it != circList.rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "\ncopy / move" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> copied(circList);
    cout << "  copy : " << copied << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> moved(std::move(copied));
    cout << "  move : " << moved << endl;

    cout << "\nconcurrencia" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> concList;
    auto worker = [&concList](int id){
        for (int i = 0; i < 1000; i++) concList.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  size = " << concList.size() << endl;

    cout << "\npersistencia (asc y desc)" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>>  cdllAsc;
    DemoList(cdllAsc,  "AscCDLL.txt");
    CircularDoubleLinkedList<DescendingDLLTrait<T1>> cdllDesc;
    DemoList(cdllDesc, "DescCDLL.txt");

    cout << "\nherencia: CDLL -> DLL -> LinkedList" << endl;
    CircularDoubleLinkedList<AscendingDLLTrait<T1>> cdll;
    cdll.insert(2, 20); cdll.insert(1, 10); cdll.insert(3, 30);
    DoubleLinkedList<AscendingDLLTrait<T1>>* pDLL = &cdll;
    LinkedList<AscendingDLLTrait<T1>>*       pLL  = &cdll;
    cout << "  size via CDLL = " << cdll.size()
         << ", via DLL* = "      << pDLL->size()
         << ", via LL* = "       << pLL->size() << endl;
    cout << "  (*pDLL)[1] = " << (*pDLL)[1] << endl;
    cout << "  loop : ";
    cdll.circularForEach(1, 1, [](T1& v){ cout << v << " "; });
    cout << endl;
}

void ListsDemo()
{
    LinkedListDemo();
    DoubleLinkedListDemo();
    CircularLinkedListDemo();
    CircularDoubleLinkedListDemo();
}
