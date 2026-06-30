#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "../types.h"
#include "BTree.h"
#include "traits.h"

using namespace std;


// (comp flexible)
using Trait = BTreeTrait<TypeBTree>;
using BT = BTree<Trait>;

// (concurrencia)
static void concurrencyWorker(BT& tree, Ref workerId)
{
    for (Size i = 0; i < 100; i++)
        tree.insert(TypeBTree('a' + ((workerId + i) % 26)), workerId);
}

void DemoBTree()
{
    // (orden flexible)
    BT bt(3);
    string keys = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";

    for (Size i = 0; i < keys.size(); i++)
        bt.insert(TypeBTree(keys[i]), Ref(i * i));

    cout << "BTree: " << bt << endl;
    cout << "size=" << bt.size()
         << " height=" << bt.height()
         << " order=" << bt.order() << endl;

    try
    {
        auto [key, ref] = bt.search('Z');
        cout << "search('Z'): key=" << key << " ref=" << ref << endl;
    }
    catch (const runtime_error& error)
    {
        cout << error.what() << endl;
    }

    Size letters = 0;
    auto countLetters = [](BT::Entry& entry, Level, Size& count) {
        if (isalpha(Byte(entry.m_data)))
            count++;
    };
    bt.forEach(countLetters, letters);
    cout << "letras=" << letters << endl;

    TypeBTree target = 'G';
    auto hasValue = [](BT::Entry& entry, Level, TypeBTree value) {
        return entry.m_data == value;
    };
    auto *found = bt.firstThat(hasValue, target);

    if (found)
        cout << "firstThat('" << target << "'): ref=" << found->m_ref << endl;

    auto [removedKey, removedRef] = bt.remove('A');
    cout << "remove('A'): key=" << removedKey
         << " ref=" << removedRef
         << " size=" << bt.size() << endl;

    string inOrder;
    for (auto& entry : bt)
        inOrder += entry.m_data;

    cout << "inorder: " << inOrder << endl;

    // (backward iterator)
    string reverseOrder;
    for (auto it = bt.rbegin(); it != bt.rend(); ++it)
        reverseOrder += it->m_data;

    cout << "reverse: " << reverseOrder << endl;

    // (operator >>)
    stringstream stream;
    stream << bt;

    BT restored(3);
    stream >> restored;
    cout << "restaurado size=" << restored.size() << endl;

    // (concurrencia)
    BT concurrentTree(3);
    vector<thread> threads;

    for (Size i = 0; i < 4; i++) {
        threads.emplace_back([&concurrentTree, i]() {
            concurrencyWorker(concurrentTree, Ref(i + 1));
        });
    }

    for (auto& thread : threads)
        thread.join();

    cout << "concurrente size=" << concurrentTree.size() << endl;
}
