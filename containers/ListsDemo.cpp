#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include"../types.h"
#include"linkedlist.h"
#include"doublelinkedlist.h"
#include"circularlinkedlist.h"
#include"circulardoublelinkedlist.h"

using namespace std;

//operator<< y operator>> deffinidos en LinkedList   heredados por todos
template <typename Container>
void DemoFileIO(Container& list, string fileName) {
    ofstream os(fileName);
    os<<list;  //operator<< heredado
    os.close();

    Container listFromFile;
    ifstream is(fileName);
    is >> listFromFile;  //operator>> heredado
    cout<<"escritura:"<<list<<endl;
    cout<<"lectura:"<<listFromFile<<endl;
}

void LinkedListDemo() {
    cout<<"\nLinkedList"<<endl;

    //Nodo LLNode<T1> ascendingTrait
    LinkedList<AscendingTrait<LLNode<T1>>> list;
    list.insert(3,30); list.insert(1,10); list.insert(2,20);

    cout<<"operator<<:"<<list<<endl; //heredado
    DemoFileIO(list,"LL.txt"); //operator>> heredado

    //LinkedListForwardIterator hereda general_iterator
    cout<<"iterator: ";
    for (auto it = list.begin(); it != list.end(); ++it)
        cout<<*it<<"";
    cout<<endl;

    LinkedList<AscendingTrait<LLNode<T1>>> lc;
    auto w = [&lc](int id){ for(int i=0;i<1000;i++) lc.push_front(i,id); };
    thread t1(w,1),t2(w,2),t3(w,3),t4(w,4),t5(w,5);
    t1.join();t2.join();t3.join();t4.join();t5.join();
    cout<<"concurrencia (esperado 5000):"<<lc.size()<<endl;
}

void DoubleLinkedListDemo() {
    cout<<"\nDoubleLinkedList"<<endl;

    //DLLNode<T1> hereda LLNode<T1> + m_prev
    DoubleLinkedList<AscendingTrait<DLLNode<T1>>> list;
    list.insert(3,30); list.insert(1,10); list.insert(5,50);
    list.insert(2,20); list.insert(4,40);

    cout<<"operator<<:"<<list<<endl; //heredado
    DemoFileIO(list,"DLL.txt"); //operator>> heredado

    //DLLBackwardIterator hereda general_iterator
    cout<<"rbegin/rend:";
    for (auto it = list.rbegin(); it != list.rend(); ++it)
        cout<<*it<<"";
    cout<<endl;
}

void CircularLinkedListDemo() {
    cout<<"\nCircularLinkedList"<<endl;

    //LLNode<T1>: mismo nodo LinkedList
    CircularLinkedList<AscendingTrait<LLNode<T1>>> list;
    list.insert(3,30); list.insert(1,10); list.insert(5,50);
    list.insert(2,20); list.insert(4,40);

    cout<<"operator<<:"<<list<<endl; //heredado
    DemoFileIO(list,"CLL.txt"); //operator>> heredado

    //CLLForwardIterator hereda circular_iterator hereda general_iterator
    cout<<"cbegin/cend:";
    for (auto it = list.cbegin(); it != list.cend(); ++it)
        cout<<*it<<"";
    cout<<endl;

    CircularLinkedList<AscendingTrait<LLNode<T1>>> c;
    c.insert(1,10); c.insert(2,20); c.insert(3,30);
    cout<<"circularForEach x2:";
    c.circularForEach(2, [](T1& v){ cout<<v<<""; });
    cout<<endl;
}

void CircularDoubleLinkedListDemo() {
    cout<<"\nCircularDoubleLinkedList"<<endl;
    //DLLNode<T1>:mismo nodo DoubleLinkedList + m_prev
    CircularDoubleLinkedList<AscendingTrait<DLLNode<T1>>> list;
    list.insert(3,30); list.insert(1,10); list.insert(5,50);
    list.insert(2,20); list.insert(4,40);

    cout<<"operator<<:"<<list<<endl; //heredado
    DemoFileIO(list,"CDLL.txt"); //operator>> heredado

    //CDLLForwardIterator hereda circular_iterator
    cout<<"cbegin/cend:  ";
    for (auto it = list.cbegin(); it != list.cend(); ++it)
        cout<<*it<<"";
    cout<<endl;

    //CDLLBackwardIterator hereda circular_iterator
    cout<<"crbegin/crend:";
    for (auto it = list.crbegin(); it != list.crend(); ++it)
        cout<<*it<<"";
    cout<<endl;

    CircularDoubleLinkedList<AscendingTrait<DLLNode<T1>>> c;
    c.insert(1,10); c.insert(2,20); c.insert(3,30);
    cout<<"circularForEach fwd x2:";
    c.circularForEach(2,  1, [](T1& v){ cout<<v<<""; });
    cout<<endl;
    cout<<"circularForEach bwd x2:";
    c.circularForEach(2, -1, [](T1& v){ cout<<v<<""; });
    cout<<endl;
}
void ListsDemo() {
    cout<<"PRUEBAS LINKEDLIST"<< endl;
    LinkedListDemo();
    cout<<endl;
    cout<<endl;
    cout<<"PRUEBAS DOUBLELINKEDLIST"<< endl;
    DoubleLinkedListDemo();
    cout<<endl;
    cout<<endl;
    cout<<"PRUEBAS CIRCULARLINKEDLIST"<< endl;
    CircularLinkedListDemo();
    cout<<endl;
    cout<<endl;
    cout<<"PRUEBAS CIRCULARDOUBLELINKEDLIST"<< endl;
    CircularDoubleLinkedListDemo();
}