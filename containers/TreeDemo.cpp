#include <iostream>
#include <fstream>
#include <thread>
#include "../types.h"
#include "BinaryTree.h"
#include "avl.h"
using namespace std;

void TreeDemo() {
    cout << "\n=== BinaryTree Demo ===" << endl;

    BinaryTree<AscendingBTTrait<T1>> tree;
    tree.insert(5, 50);
    tree.insert(3, 30);
    tree.insert(7, 70);
    tree.insert(1, 10);
    tree.insert(4, 40);
    tree.insert(6, 60);
    tree.insert(8, 80);

    cout << "operator<<:  " << tree << endl;
    cout << tree.toString() << endl;

    cout << "--- printTree ---" << endl;
    tree.printTree();

    auto [d, r] = tree.search(4);
    cout << "search(4) -> dato: " << d << " ref: " << r << endl;

    cout << "inorder forEach:  ";
    tree.inorder().forEach([](auto& x){ cout << x << " "; });
    cout << endl;

    cout << "preorder forEach: ";
    tree.preorder().forEach([](auto& x){ cout << x << " "; });
    cout << endl;

    cout << "postorder forEach: ";
    tree.postorder().forEach([](auto& x){ cout << x << " "; });
    cout << endl;

    cout << "inorder rForEach (rev): ";
    tree.inorder().rForEach([](auto& x){ cout << x << " "; });
    cout << endl;

    cout << "range-based for (inorder): ";
    for(auto& x : tree) cout << x << " ";
    cout << endl;

    cout << "ForEach (*2): ";
    tree.ForEach([](auto& x){ cout << x*2 << " "; });
    cout << endl;

    // operator>> — persistencia
    ofstream os("BT.txt");
    os << tree << endl;
    BinaryTree<AscendingBTTrait<T1>> tree2;
    ifstream is("BT.txt");
    is >> tree2;
    cout << "leida desde archivo: " << tree2 << endl;

    // copy constructor
    BinaryTree<AscendingBTTrait<T1>> copia(tree);
    cout << "copia:  " << copia << endl;

    // move constructor
    BinaryTree<AscendingBTTrait<T1>> movido(move(copia));
    cout << "movido: " << movido << endl;
    cout << "copia vacia: " << copia << endl;

    // ─── AVL Demo ───────────────────────────────────────────────────────────
    cout << "\n=== AVL Demo ===" << endl;

    // Ascending AVL — inserciones en orden que degeneraría un BST normal
    AVL<AscendingAVLTrait<T1>> avl;
    for(T1 v : {4, 2, 6, 1, 3, 5, 7}) avl.insert(v, v*10);
    cout << "insert 4,2,6,1,3,5,7:" << endl;
    avl.printTree();
    cout << "operator<<:  " << avl << endl;
    cout << "height: " << avl.height() << " | balance: " << avl.balance() << endl;

    // Caso degenerado en BST normal → AVL mantiene altura ≈ log2(n)
    AVL<AscendingAVLTrait<T1>> avl2;
    for(T1 v = 1; v <= 7; ++v) avl2.insert(v, v*10);
    cout << "\ninsert 1..7 (degeneraría BST normal):" << endl;
    avl2.printTree();
    cout << "height: " << avl2.height() << " (BST normal daría 7)" << endl;

    // Traversals heredados de BinaryTree
    cout << "inorder:  "; avl.inorder().forEach([](auto& x){ cout << x << " "; }); cout << endl;
    cout << "preorder: "; avl.preorder().forEach([](auto& x){ cout << x << " "; }); cout << endl;

    // search heredado
    auto [d2, r2] = avl.search(3);
    cout << "search(3) -> dato: " << d2 << " ref: " << r2 << endl;

    // operator<< / operator>> heredados
    ofstream os2("AVL.txt");
    os2 << avl << endl;
    AVL<AscendingAVLTrait<T1>> avl3;
    ifstream is2("AVL.txt");
    is2 >> avl3;
    cout << "leida desde archivo: " << avl3 << endl;
    cout << "height tras reconstruir: " << avl3.height() << endl;

    // copy constructor
    AVL<AscendingAVLTrait<T1>> avlCopia(avl);
    cout << "copia:  " << avlCopia << " height: " << avlCopia.height() << endl;

    // Concurrencia — 4 hilos insertan en paralelo
    AVL<AscendingAVLTrait<T1>> avlConc;
    {
        auto worker = [&avlConc](int start) {
            for(int i = start; i < start + 25; ++i)
                avlConc.insert(i, i);
        };
        thread t1(worker, 0),  t2(worker, 25),
               t3(worker, 50), t4(worker, 75);
        t1.join(); t2.join(); t3.join(); t4.join();
    }
    cout << "\nConcurrencia: 4 hilos x 25 inserts = 100 nodos" << endl;
    size_t concSize = 0;
    avlConc.ForEach([&concSize](auto&){ ++concSize; });
    cout << "size (ForEach count): " << concSize << endl;
    cout << "height: " << avlConc.height() << " (log2(100) ≈ 7)" << endl;
    cout << "balance raiz [-1,1]: " << avlConc.balance() << endl;
}
