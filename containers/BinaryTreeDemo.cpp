#include <iostream>
#include <fstream>
#include <thread>
#include <vector>

#include "../types.h"
#include "BinaryTree.h"

using namespace std;

void BinaryTreeDemo() {
    cout << "\n--- BinaryTree ---" << endl;

    cout << "\ninsercion ordenada" << endl;
    BinaryTree<AscendingBTTrait<T1>> asc;
    for (T1 v : {28, 17, 8, 4, 35}) asc.insert(v, v * 10);
    cout << "  asc  : " << asc << endl;
    cout << "  asc  toString : " << asc.toString() << endl;

    BinaryTree<DescendingBTTrait<T1>> desc;
    for (T1 v : {28, 17, 8, 4, 35}) desc.insert(v, v * 10);
    cout << "  desc : " << desc << endl;

    cout << "\npersistencia" << endl;
    {
        ofstream os("BT.txt");
        os << asc << endl;
    }
    BinaryTree<AscendingBTTrait<T1>> fromFile;
    {
        ifstream is("BT.txt");
        is >> fromFile;
    }
    cout << "  guardado : " << asc      << endl;
    cout << "  leido    : " << fromFile << endl;
    cout << "  igual ? " << (asc.toString() == fromFile.toString()) << endl;

    cout << "\nranged-for (inorder)" << endl;
    cout << "  ";
    for (auto& v : asc) cout << v << " ";
    cout << endl;

    cout << "\niteradores" << endl;
    BinaryTree<AscendingBTTrait<T1>> t;
    for (T1 v : {5, 3, 7, 1, 4, 6, 8}) t.insert(v, v * 10);

    cout << "  inorder  fwd : ";
    for (auto it = t.begin();      it != t.end();      ++it) cout << *it << " ";
    cout << "\n  inorder  bwd : ";
    for (auto it = t.rbegin();     it != t.rend();     ++it) cout << *it << " ";
    cout << "\n  preorder fwd : ";
    for (auto it = t.pre_begin();  it != t.pre_end();  ++it) cout << *it << " ";
    cout << "\n  preorder bwd : ";
    for (auto it = t.pre_rbegin(); it != t.pre_rend(); ++it) cout << *it << " ";
    cout << "\n  postord  fwd : ";
    for (auto it = t.post_begin(); it != t.post_end(); ++it) cout << *it << " ";
    cout << "\n  postord  bwd : ";
    for (auto it = t.post_rbegin();it != t.post_rend();++it) cout << *it << " ";
    cout << endl;

    cout << "\ncopy / move" << endl;
    BinaryTree<AscendingBTTrait<T1>> copied(t);
    cout << "  copy : " << copied << endl;
    BinaryTree<AscendingBTTrait<T1>> moved(move(copied));
    cout << "  move : " << moved << " (origen size=" << copied.size() << ")" << endl;

    cout << "\nsize / height" << endl;
    cout << "  size   = " << t.size()   << endl;
    cout << "  height = " << t.height() << endl;

    cout << "\nconcurrencia (5 hilos x 1000)" << endl;
    BinaryTree<AscendingBTTrait<T1>> concTree;
    auto worker = [&concTree](int id) {
        for (int i = 0; i < 1000; i++) concTree.insert(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  size = " << concTree.size() << endl;

    cout << "\nlevel-order (BFS)" << endl;
    BinaryTree<AscendingBTTrait<T1>> bfs;
    for (T1 v : {4, 2, 6, 1, 3, 5, 7}) bfs.insert(v, v * 10);
    cout << "  inorder : ";
    for (auto it = bfs.begin(); it != bfs.end(); ++it) cout << *it << " ";
    cout << "\n  level   : ";
    for (auto it = bfs.level_begin(); it != bfs.level_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "\nprint2D" << endl;
    BinaryTree<AscendingBTTrait<T1>> vis;
    for (T1 v : {5, 3, 7, 1, 4, 6, 8}) vis.insert(v, v * 10);
    vis.print2D();
}
