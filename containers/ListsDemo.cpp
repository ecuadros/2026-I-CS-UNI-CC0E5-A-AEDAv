#include "linkedlist.h"
#include <sstream>

void LinkedListDemo()
{
    LinkedList<DescendingLinkedListTrait<T1>> list;
    list.insert(3, 0);
    list.insert(1, 0);
    list.insert(5, 0);
    list.insert(2, 0);
    cout << "insert:      " << list << endl; // [5 -> 3 -> 2 -> 1]

    LinkedList<AscendingLinkedListTrait<T1>> list2;
    list2.push_back(2, 0);
    list2.push_back(3, 0);
    list2.push_front(1, 0);
    cout << "push:        " << list2 << endl; // [1 -> 2 -> 3]

    list2.pop_front();
    cout << "pop_front:   " << list2 << endl; // [2 -> 3]
    list2.pop_back();
    cout << "pop_back:    " << list2 << endl; // [2]

    LinkedList<AscendingLinkedListTrait<T1>> list3;
    list3.push_back(10, 0);
    list3.push_back(20, 0);
    list3.push_back(30, 0);
    cout << "size:        " << list3.size() << endl;
    cout << "operator[1]: " << list3[1] << endl;

    LinkedList<AscendingLinkedListTrait<T1>> list4;
    istringstream input("[5 -> 10 -> 15]");
    input >> list4;
    cout << "operator>>:  " << list4 << endl; // [5 -> 10 -> 15]

    // ForEach escritura
    list3.ForEach([](T1 &val)
                  { val *= 2; });
    cout << "ForEach*2:   " << list3 << endl; // [20 -> 40 -> 60]

    // ForEach lectura
    const auto &clist = list3;
    clist.ForEach([](T1 val)
                  { cout << val << " "; });
    cout << endl;

    LinkedList<AscendingLinkedListTrait<T1>> list5 = list3;
    cout << "copy:        " << list5 << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list6 = std::move(list5);
    cout << "move:        " << list6 << endl;
    cout << "moved-from:  " << list5 << endl;
}

void ListsDemo()
{
    LinkedListDemo();
}
