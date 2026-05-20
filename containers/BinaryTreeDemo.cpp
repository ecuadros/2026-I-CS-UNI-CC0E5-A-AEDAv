#include <iostream>
#include <fstream>
#include <thread>
#include <vector>

#include "../types.h"
#include "BinaryTree.h"

using namespace std;

void BinaryTreeDemo() {
    cout << "==========================================================" << endl;
    cout << "                  DEMO: BINARY TREE                       " << endl;
    cout << "==========================================================" << endl;

    cout << "\n1. INSERCION ORDENADA (Ascendente y Descendente)" << endl;
    BinaryTree<AscendingBTTrait<T1>> asc;
    for (T1 v : {28, 17, 8, 4, 35}) asc.insert(v, v * 10);
    cout << "  Asc  operator<<: " << asc << endl;
    cout << "  Asc  toString(): " << asc.toString() << endl;

    BinaryTree<DescendingBTTrait<T1>> desc;
    for (T1 v : {28, 17, 8, 4, 35}) desc.insert(v, v * 10);
    cout << "  Desc operator<<: " << desc << endl;

    cout << "\n2. PERSISTENCIA A ARCHIVOS (operator<< / operator>>)" << endl;
    {
        ofstream os("BT.txt");
        os << asc << endl;
    }
    BinaryTree<AscendingBTTrait<T1>> fromFile;
    {
        ifstream is("BT.txt");
        is >> fromFile;
    }
    cout << "  Guardado:  " << asc      << endl;
    cout << "  Leido:     " << fromFile << endl;
    cout << "  ESTADO: " << (asc.toString() == fromFile.toString() ? "[EXITO]" : "[FALLO]") << endl;

    cout << "\n3. RANGED-FOR NATIVO (inorder forward iterator)" << endl;
    cout << "  for(auto& v : tree): ";
    for (auto& v : asc) cout << v << " ";
    cout << endl;

    cout << "\n4. SEIS ITERADORES" << endl;
    BinaryTree<AscendingBTTrait<T1>> t;
    for (T1 v : {5, 3, 7, 1, 4, 6, 8}) t.insert(v, v * 10);

    cout << "  Inorder    fwd (L,N,R): ";
    for (auto it = t.begin();      it != t.end();      ++it) cout << *it << " ";
    cout << endl;

    cout << "  Inorder    bwd (R,N,L): ";
    for (auto it = t.rbegin();     it != t.rend();     ++it) cout << *it << " ";
    cout << endl;

    cout << "  Preorder   fwd (N,L,R): ";
    for (auto it = t.pre_begin();  it != t.pre_end();  ++it) cout << *it << " ";
    cout << endl;

    cout << "  Preorder   bwd (N,R,L): ";
    for (auto it = t.pre_rbegin(); it != t.pre_rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "  Postorder  fwd (L,R,N): ";
    for (auto it = t.post_begin(); it != t.post_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "  Postorder  bwd (N,R,L): ";
    for (auto it = t.post_rbegin();it != t.post_rend();++it) cout << *it << " ";
    cout << endl;

    cout << "\n5. ForEach (inorder)" << endl;
    cout << "  ForEach: ";
    t.ForEach([](T1& v) { cout << v << " "; });
    cout << endl;

    cout << "\n6. BIG FIVE: COPY Y MOVE CONSTRUCTORS" << endl;
    BinaryTree<AscendingBTTrait<T1>> copied(t);
    cout << "  Copia  (Copy Ctor): " << copied << endl;
    BinaryTree<AscendingBTTrait<T1>> moved(move(copied));
    cout << "  Movido (Move Ctor): " << moved << endl;
    cout << "  Origen tras move (size): " << copied.size() << endl;

    cout << "\n7. height() y size()" << endl;
    cout << "  size()  : " << t.size()   << endl;
    cout << "  height(): " << t.height() << endl;

    cout << "\n8. CONCURRENCIA (5 hilos x 1000 inserts)" << endl;
    BinaryTree<AscendingBTTrait<T1>> concTree;
    auto worker = [&concTree](int id) {
        for (int i = 0; i < 1000; i++) concTree.insert(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado 5000 | Actual: " << concTree.size() << endl;
    cout << "  ESTADO: " << (concTree.size() == 5000 ? "[EXITO]" : "[FALLO]") << endl;

    cout << "\n9. MEJORA #1: Level-order iterator (BFS - por niveles)" << endl;
    BinaryTree<AscendingBTTrait<T1>> bfs;
    for (T1 v : {4, 2, 6, 1, 3, 5, 7}) bfs.insert(v, v * 10);
    cout << "  Inorder (referencia): ";
    for (auto it = bfs.begin(); it != bfs.end(); ++it) cout << *it << " ";
    cout << endl;
    cout << "  Level-order (BFS)  : ";
    for (auto it = bfs.level_begin(); it != bfs.level_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "\n10. MEJORA #2: print2D - vista ASCII del arbol rotado 90 grados" << endl;
    BinaryTree<AscendingBTTrait<T1>> vis;
    for (T1 v : {5, 3, 7, 1, 4, 6, 8}) vis.insert(v, v * 10);
    cout << "  (derecha=arriba, izquierda=abajo)" << endl;
    vis.print2D();
}
