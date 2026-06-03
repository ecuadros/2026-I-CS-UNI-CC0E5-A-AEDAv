#include <iostream>
#include <sstream>
#include <utility>
#include "hashtable.h"
using namespace std;

void HashTableDemo() {
    cout << "\nDemo HashTable (AVL)" << endl;
    HashTable<int, int> table(7);

    table.insert(42, 7081);
    table.insert(119, -36);
    table.insert(203, 945);
    table.insert(56, 12012);
    table.insert(77, 301);
    table[5] = 3;
    table[14] = 8088;
    table[119] = -41;

    cout << table << endl;
    cout << "table[5]: " << table.at(5) << endl;
    cout << "table[14]: " << table.at(14) << endl;
    cout << "contains(203): " << table.contains(203) << endl;

    cout << "items: ";
    for (const auto &[key, value] : table) {
        cout << "(" << key << "," << value << ") ";
    }
    cout << endl;

    HashTable<int, int> copy(table);
    HashTable<int, int> moved(std::move(copy));
    cout << "moved copy: " << moved << endl;

    stringstream input("[(301,234),(58,653453),(-17,11),(912,2334),(44,-606)]");
    HashTable<int, int> fromStream(7);
    input >> fromStream;
    cout << "from stream: " << fromStream << endl;
}
