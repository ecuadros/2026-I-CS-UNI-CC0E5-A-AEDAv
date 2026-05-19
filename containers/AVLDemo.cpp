#include <iostream>
#include <thread>

#include "../types.h"
#include "AVL.h"
#include "BinaryTree.h"

using namespace std;

void AVLDemo() {
    cout << "\n--- AVL ---" << endl;

    cout << "\nBST vs AVL con insercion ordenada" << endl;
    BinaryTree<AscendingBTTrait<T1>> bst;
    AVL<AscendingAVLTrait<T1>>       avl;
    for (T1 v : {1, 2, 3, 4, 5, 6, 7}) {
        bst.insert(v, v * 10);
        avl.insert(v, v * 10);
    }
    cout << "  BST : " << bst << endl;
    cout << "  AVL : " << avl << endl;
    cout << "  altura BST = 7, altura AVL = " << avl.height() << endl;

    cout << "\nrotaciones" << endl;

    cout << "  LL (30,20,10):" << endl;
    AVL<DescendingAVLTrait<T1>> ll;
    ll.insert(30,3); ll.insert(20,2); ll.insert(10,1);
    cout << "    " << ll << "  h=" << ll.height() << "  bf=" << ll.balanceFactor() << endl;

    cout << "  RR (10,20,30):" << endl;
    AVL<DescendingAVLTrait<T1>> rr;
    rr.insert(10,1); rr.insert(20,2); rr.insert(30,3);
    cout << "    " << rr << "  h=" << rr.height() << "  bf=" << rr.balanceFactor() << endl;

    cout << "  LR (30,10,20):" << endl;
    AVL<DescendingAVLTrait<T1>> lr;
    lr.insert(30,3); lr.insert(10,1); lr.insert(20,2);
    cout << "    " << lr << "  h=" << lr.height() << "  bf=" << lr.balanceFactor() << endl;

    cout << "  RL (10,30,20):" << endl;
    AVL<DescendingAVLTrait<T1>> rl;
    rl.insert(10,1); rl.insert(30,3); rl.insert(20,2);
    cout << "    " << rl << "  h=" << rl.height() << "  bf=" << rl.balanceFactor() << endl;

    cout << "\naltura con 1024 inserciones" << endl;
    AVL<AscendingAVLTrait<T1>> big;
    for (T1 i = 1; i <= 1024; ++i) big.insert(i, i);
    cout << "  size   = " << big.size()   << endl;
    cout << "  height = " << big.height() << endl;

    cout << "\niteradores heredados" << endl;
    AVL<DescendingAVLTrait<T1>> t;
    for (T1 v : {5, 3, 7, 1, 4, 6, 8}) t.insert(v, v * 10);
    cout << "  inorder  : ";
    for (auto it = t.begin();      it != t.end();      ++it) cout << *it << " ";
    cout << "\n  preorder : ";
    for (auto it = t.pre_begin();  it != t.pre_end();  ++it) cout << *it << " ";
    cout << "\n  postord  : ";
    for (auto it = t.post_begin(); it != t.post_end();++it) cout << *it << " ";
    cout << endl;

    cout << "\nconcurrencia (5 hilos x 1000)" << endl;
    AVL<AscendingAVLTrait<T1>> conc;
    auto worker = [&conc](T1 id) {
        for (T1 i = 0; i < 1000; ++i) conc.insert(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  size   = " << conc.size()   << endl;
    cout << "  height = " << conc.height() << endl;
}
