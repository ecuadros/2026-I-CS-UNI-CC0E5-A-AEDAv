#include "linkedlist.h"

void LinkedListDemo(){
 LinkedList<DescendingLinkedListTrait<T1>> list;
    list.insert(1, 15);
    list.insert(2, 25);
    list.insert(3, 35);
    list.insert(4, 45);
    list.insert(5, 55);
    cout << list << endl;
    list.push_front(8, 76);
    list.push_back(76, 778);
    cout << list << endl;
    list.pop_front();
    list.pop_back();
    cout<<list<<endl;
    cout<<list[2]<<endl;
}

void ListsDemo(){
    LinkedListDemo();
    
}
