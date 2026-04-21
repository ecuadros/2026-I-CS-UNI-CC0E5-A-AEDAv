#include "linkedlist.h"
#include <fstream>
void LinkedListDemo(){
    LinkedList< DescendingLinkedListTrait<T1> > list;
    list.insert(1, 15);
    list.insert(2, 25);
    list.insert(3, 35);
    list.insert(4, 45);
    list.insert(5, 55);
    cout << "insert:" << list << endl;

    list.push_front(6, 65);
    cout << "push_front:" << list << endl;

    list.pop_front();
    cout << "pop_front:" << list << endl;

    list.push_back(0, 5);
    cout << "push_back:" << list << endl;

    list.pop_back();
    cout << "pop_back:" << list << endl;

    cout << "lista[4]:" << list[4] << endl;

    list.ForEach([](auto& x){ cout << x << " "; });
    cout << endl;

    ofstream of("linkedlist.txt");
    of << list << endl;

}

void ListsDemo(){
    LinkedListDemo();
    
}
