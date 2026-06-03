#include <iostream>
#include <fstream>
#include <thread>
#include "../types.h"
#include "BinaryTree.h"
#include "avl.h"
#include "hashtable.h"
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
    tree.printTree(cout);

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

    cout << "rinorder:   "; for(auto& x : tree.rinorder())   cout << x << " "; cout << endl;
    cout << "rpreorder:  "; for(auto& x : tree.rpreorder())  cout << x << " "; cout << endl;
    cout << "rpostorder: "; for(auto& x : tree.rpostorder()) cout << x << " "; cout << endl;
    cout << "toString(PREORDER):  " << tree.toString(Traversal::PREORDER)  << endl;
    cout << "toString(POSTORDER): " << tree.toString(Traversal::POSTORDER) << endl;

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
    avl.printTree(cout);
    cout << "operator<<:  " << avl << endl;
    cout << "height: " << avl.height() << " | balance: " << avl.balance() << endl;

    // Caso degenerado en BST normal
    AVL<AscendingAVLTrait<T1>> avl2;
    for(T1 v = 1; v <= 7; ++v) avl2.insert(v, v*10);
    cout << "\ninsert 1..7 (degeneraría BST normal):" << endl;
    avl2.printTree(cout);
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

    // HashTable Demo
    cout << "\n=== HashTable Demo ===" << endl;
    HashTable<AscendingAVLTrait<T1>> m;
    m[5] = 3;
    m[2] = 7;
    m[8] = 1;
    m[2] = 9;   // actualiza la clave existente
    cout << "operator<<: " << m << endl;

    cout << "[key,value]: ";
    for(const auto& [k, v] : m) cout << "(" << k << "->" << v << ") ";
    cout << endl;

    // persistencia round-trip
    ofstream osh("HASH.txt"); osh << m << endl;
    HashTable<AscendingAVLTrait<T1>> m2;
    ifstream ish("HASH.txt"); ish >> m2;
    cout << "leida desde archivo: " << m2 << endl;

    // copy independiente
    HashTable<AscendingAVLTrait<T1>> m3(m);
    m3[5] = 99;
    cout << "original m[5]=" << m[5] << " | copia m3[5]=" << m3[5] << endl;

    // move
    HashTable<AscendingAVLTrait<T1>> m4(move(m3));
    cout << "movida: " << m4 << endl;
}
