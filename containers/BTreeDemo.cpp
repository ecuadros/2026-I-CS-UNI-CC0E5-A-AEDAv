#include <cctype>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "../types.h"
#include "BTree.h"
#include "DemoUtils.h"
#include "traits.h"

using namespace std;

// Arbol 6 7
using Trait = BTreeTrait<TypeBTree, 6>;
using BT = BTree<Trait>;

static void concurrencyWorker(BT& tree, Ref workerId) {
    for (Size i = 0; i < 200; ++i)
        tree.insert(TypeBTree('a' + ((workerId * 7 + i) % 26)), workerId);
}

void DemoBTree() {
    printTitle("BTREE", "==========");

    // insert
    printTitle("insert");
    BT bt;
    const string keys =
        "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";

    for (Size i = 0; i < keys.size(); ++i)
        bt.insert(keys[i], Ref(i * i));

    cout << "  size=" << bt.size()
         << "  height=" << bt.height()
         << "  order=" << bt.order() << '\n';

    // search
    printTitle("search");
    try {
        auto [value, ref] = bt.search('Z');
        cout << "  search('Z') -> encontrado  valor="
             << value << "  ref=" << ref << '\n';
    } catch (const runtime_error& e) {
        cout << "  search('Z') -> " << e.what() << '\n';
    }

    try {
        bt.search('!');
    } catch (const runtime_error& e) {
        cout << "  search('!') -> " << e.what() << '\n';
    }

    // ForEach variadic
    printTitle("forEach variadic");
    Size letterCount = 0;
    bt.forEach(
        [](BT::Entry& e, Level, Size& count) {
            if (isalpha(static_cast<Byte>(e.m_data)))
                ++count;
        },
        letterCount);
    cout << "  letras en el arbol: " << letterCount << '\n';

    // FirstThat variadic
    printTitle("firstThat variadic");
    auto* entry = bt.firstThat(
        [](BT::Entry& e, Level, TypeBTree target) {
            return e.m_data == target;
        },
        TypeBTree('M'));

    cout << "  firstThat('M') -> "
         << (entry ? "encontrado" : "no encontrado");
    if (entry)
        cout << " ref=" << entry->m_ref;
    cout << '\n';

    // remove
    printTitle("remove");
    Size beforeRemove = bt.size();
    auto [removedValue, removedRef] = bt.remove('A');
    cout << "  remove('A') -> eliminado: valor="
         << removedValue << " ref=" << removedRef
         << "  size antes=" << beforeRemove
         << "  size despues=" << bt.size() << '\n';

    // iterator
    printTitle("for (auto& entry : bt) - iterador inorder");
    string inorderKeys;
    for (auto& e : bt)
        inorderKeys += e.m_data;
    cout << "  claves en orden: " << inorderKeys << '\n';

    // useCount
    printTitle("useCount() - contador de accesos por clave");
    bt.search('B'); bt.search('B'); bt.search('B');
    bt.search('C');
    for (auto& e : bt)
        if (e.m_data == 'B' || e.m_data == 'C')
            cout << "  '" << e.m_data << "' useCount=" << e.useCount() << '\n';

    testIO(bt);
    testCopyMove(bt, [](BT& copy) { copy.insert('!', 999); });

    // concurrencia
    printTitle("Concurrencia");
    BT concurrentTree;
    const Size threadCount = 5;
    const Size insertsPerThread = 200;
    vector<thread> threads;
    threads.reserve(threadCount);

    for (Size i = 0; i < threadCount; ++i)
        threads.emplace_back(
            concurrencyWorker,
            ref(concurrentTree),
            Ref(i + 1));

    for (auto& t : threads) t.join();

    cout << "  inserciones concurrentes lanzadas: "
         << threadCount * insertsPerThread << '\n';
    cout << "  size final (sin corrupcion, <= 26 claves unicas): "
         << concurrentTree.size() << '\n';

    printTitle("FIN BTREE", "==========");
}
