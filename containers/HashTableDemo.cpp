#include <iostream>
#include <initializer_list>
#include <sstream>
#include <utility>
#include "hashtable.h"
#include "../types.h"
using namespace std;

using DemoHashTable = HashTable<T1, T1>;

template <typename Table>
void InsertHashItems(Table &table, initializer_list<pair<typename Table::key_type, typename Table::mapped_type>> items) {
    for (const auto &[key, value] : items) {
        table.insert(key, value);
    }
}

template <typename Table>
void PrintHashLookup(const Table &table, const typename Table::key_type &key) {
    cout << "table[" << key << "]: " << table.at(key) << endl;
}

template <typename Table>
void PrintHashItems(const Table &table) {
    cout << "items: ";
    for (const auto &[key, value] : table) {
        cout << "(" << key << "," << value << ") ";
    }
    cout << endl;
}

void HashTableDemo() {
    cout << "\nDemo HashTable (AVL)" << endl;
    DemoHashTable table(7);

    InsertHashItems(table, {{42, 7081}, {119, -36}, {203, 945}, {56, 12012}, {77, 301}});
    table[5] = 3;
    table[14] = 8088;
    table[119] = -41;

    cout << table << endl;
    PrintHashLookup(table, 5);
    PrintHashLookup(table, 14);
    cout << "contains(203): " << table.contains(203) << endl;
    PrintHashItems(table);

    DemoHashTable copy(table);
    DemoHashTable moved(std::move(copy));
    cout << "moved copy: " << moved << endl;

    stringstream input("[(301,234),(58,653453),(-17,11),(912,2334),(44,-606)]");
    DemoHashTable fromStream(7);
    input >> fromStream;
    cout << "from stream: " << fromStream << endl;
}
