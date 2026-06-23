// P1 Tarea Adaptar Demo

#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <string>
#include "../types.h"
#include "BTree.h"
#include "traits.h"
using namespace std;

using TypeBTree = char;

// Worker para prueba de concurrencia
template <typename BT>
static void concurrencyWorker(BT& tree, Ref workerId) {
    for (Size i = 0; i < 200; ++i)
        tree.insert(TypeBTree('a' + ((workerId * 7 + i) % 26)), workerId);
}

// Pruebas genericas para cualquier BTree<Trait>
template <typename BT>
static void runTests(BT& bt, const string& treeName) {
    cout << "\n===== " << treeName << " =====\n";

    // insert
    cout << "\n[insert]\n";
    const string keys = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    for (Size i = 0; i < keys.size(); ++i)
        bt.insert(keys[i], Ref(i * i));
    cout << "  size=" << bt.size() << "  height=" << bt.height() << "  order=" << bt.order() << "\n";

    // search
    cout << "\n[search]\n";
    try {
        auto [val, ref] = bt.search('Z');
        cout << "  search('Z') -> encontrado  valor=" << val << "  ref=" << ref << "\n";
    } catch (const runtime_error& e) {
        cout << "  search('Z') -> " << e.what() << "\n";
    }
    try {
        bt.search('!');
    } catch (const runtime_error& e) {
        cout << "  search('!') -> " << e.what() << "\n";
    }

    // P1 Tarea ForEach Variadic: forEach con lambda y argumento extra
    cout << "\n[forEach variadic]\n";
    Size letterCount = 0;
    bt.forEach([](typename BT::Entry& e, Depth, Size& count) {
        if (isalpha(Byte(e.m_data))) ++count;
    }, letterCount);
    cout << "  letras en el arbol: " << letterCount << "\n";

    // P1 Tarea ForEach Variadic: firstThat con lambda y argumento extra
    cout << "\n[firstThat variadic]\n";
    auto* entry = bt.firstThat([](typename BT::Entry& e, Depth, TypeBTree target) {
        return e.m_data == target;
    }, TypeBTree('M'));
    cout << "  firstThat('M') -> " << (entry ? "encontrado" : "no encontrado");
    if (entry) cout << " ref=" << entry->m_ref;
    cout << "\n";

    // remove
    cout << "\n[remove]\n";
    Size beforeRemove = bt.size();
    auto [removedVal, removedRef] = bt.remove('A');
    cout << "  remove('A') -> eliminado: valor=" << removedVal << " ref=" << removedRef
         << "  size antes=" << beforeRemove << "  size despues=" << bt.size() << "\n";

    // iterador begin/end
    cout << "\n[for (auto& e : bt) - iterador inorder]\n";
    string inorderKeys;
    for (auto& e : bt) inorderKeys += e.m_data;
    cout << "  claves en orden: " << inorderKeys << "\n";

    // useCount
    cout << "\n[useCount]\n";
    bt.search('B'); bt.search('B'); bt.search('B');
    bt.search('C');
    for (auto& e : bt)
        if (e.m_data == 'B' || e.m_data == 'C')
            cout << "  '" << e.m_data << "' useCount=" << e.useCount() << "\n";

    // operator<< / operator>>
    cout << "\n[operator<< / operator>>]\n";
    ostringstream oss;
    oss << bt;
    cout << "  Serializado : " << oss.str() << "\n";
    BT bt2;
    istringstream iss(oss.str());
    iss >> bt2;
    cout << "  Deserializado: " << bt2 << "\n";

    // copy constructor
    cout << "\n[Copy constructor]\n";
    BT copia(bt);
    copia.insert('!', 999);
    cout << "  Original : size=" << bt.size()    << "  " << bt    << "\n";
    cout << "  Copia    : size=" << copia.size() << "  " << copia << "\n";

    // move constructor
    cout << "\n[Move constructor]\n";
    BT movida(move(copia));
    cout << "  Movida   : size=" << movida.size() << "  " << movida << "\n";
    cout << "  Fuente tras move: size=" << copia.size() << "\n";

    // concurrencia
    cout << "\n[Concurrencia]\n";
    BT concurrentTree;
    const Size kThreads = 5;
    vector<thread> threads;
    threads.reserve(kThreads);
    for (Size i = 0; i < kThreads; ++i)
        threads.emplace_back(concurrencyWorker<BT>, std::ref(concurrentTree), Ref(i + 1));
    for (auto& t : threads) t.join();
    cout << "  size final (<= 26 claves unicas): " << concurrentTree.size() << "\n";

    cout << "\n===== FIN " << treeName << " =====\n";
}

void DemoBTree23() {
    cout << "\n             PRUEBAS BTREE 2-3             \n";
    BTree<Tree23Trait<TypeBTree>> bt;
    runTests(bt, "BTree 2-3");
}

void DemoBTree34() {
    cout << "\n             PRUEBAS BTREE 3-4             \n";
    BTree<Tree34Trait<TypeBTree>> bt;
    runTests(bt, "BTree 3-4");
}
