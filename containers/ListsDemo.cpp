#include "linkedlist.h"
#include <sstream>

void LinkedListDemo(){
    // T4, T11: insert ordenado + operator<<
    LinkedList<DescendingLinkedListTrait<T1>> list;
    list.insert(1, 15);
    list.insert(2, 25);
    list.insert(3, 35);
    list.insert(4, 45);
    list.insert(5, 55);
    cout << "insert (desc): " << list << endl;

    // T10: size
    cout << "size: " << list.size() << endl;

    // T9: operator[]
    cout << "[0]: " << list[0] << "  [4]: " << list[4] << endl;

    // T5, T6: push_front / pop_front
    list.push_front(10, 100);
    cout << "push_front(10): " << list << endl;
    list.pop_front();
    cout << "pop_front:      " << list << endl;

    // T7, T8: push_back / pop_back
    list.push_back(0, 0);
    cout << "push_back(0):   " << list << endl;
    list.pop_back();
    cout << "pop_back:       " << list << endl;

    // T1: copy constructor
    LinkedList<DescendingLinkedListTrait<T1>> copy(list);
    cout << "copy:           " << copy << endl;

    // T2: move constructor
    LinkedList<DescendingLinkedListTrait<T1>> moved(std::move(copy));
    cout << "moved:          " << moved << endl;
    cout << "copy (vaciado): " << copy  << endl;

    // T12: operator>>
    LinkedList<AscendingLinkedListTrait<T1>> list2;
    istringstream iss("[10,20,30]");
    iss >> list2;
    cout << "operator>>:     " << list2 << endl;
}

void ListsDemo(){
    LinkedListDemo();
}
