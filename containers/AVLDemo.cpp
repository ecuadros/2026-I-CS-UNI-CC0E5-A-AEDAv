#include <iostream>
#include <thread>

#include "../types.h"
#include "AVL.h"
#include "BinaryTree.h"

using namespace std;

// ToDO comparar la altura de BST vs AVL para una insercion patologica (orden creciente)
void AVLDemo() {
    cout << "==========================================================" << endl;
    cout << "                     DEMO: AVL TREE                       " << endl;
    cout << "==========================================================" << endl;

    cout << "\n1. INSERCION BALANCEADA (caso patologico: ordenado)" << endl;
    BinaryTree<AscendingBTTrait<T1>> bst;
    AVL<AscendingAVLTrait<T1>>       avl;
    for (T1 v : {1, 2, 3, 4, 5, 6, 7}) {
        bst.insert(v, v * 10);
        avl.insert(v, v * 10);
    }
    cout << "  BST (degenerado): " << bst << endl;
    cout << "  AVL (balanceado): " << avl << endl;
    cout << "  Altura BST: 7 (lineal)   |  Altura AVL: " << avl.height() << endl;

    cout << "\n2. ROTACIONES (verificando los 4 casos)" << endl;

    cout << "  Caso LL (inserciones 30,20,10 = bf>1, child bf>0)" << endl;
    AVL<DescendingAVLTrait<T1>> ll;
    ll.insert(30,3); ll.insert(20,2); ll.insert(10,1);
    cout << "    Resultado: " << ll << " | h=" << ll.height() << " | bf=" << ll.balanceFactor() << endl;

    cout << "  Caso RR (inserciones 10,20,30 = bf<-1, child bf<0)" << endl;
    AVL<DescendingAVLTrait<T1>> rr;
    rr.insert(10,1); rr.insert(20,2); rr.insert(30,3);
    cout << "    Resultado: " << rr << " | h=" << rr.height() << " | bf=" << rr.balanceFactor() << endl;

    cout << "  Caso LR (inserciones 30,10,20 = bf>1, child bf<0)" << endl;
    AVL<DescendingAVLTrait<T1>> lr;
    lr.insert(30,3); lr.insert(10,1); lr.insert(20,2);
    cout << "    Resultado: " << lr << " | h=" << lr.height() << " | bf=" << lr.balanceFactor() << endl;

    cout << "  Caso RL (inserciones 10,30,20 = bf<-1, child bf>0)" << endl;
    AVL<DescendingAVLTrait<T1>> rl;
    rl.insert(10,1); rl.insert(30,3); rl.insert(20,2);
    cout << "    Resultado: " << rl << " | h=" << rl.height() << " | bf=" << rl.balanceFactor() << endl;

    cout << "\n3. ALTURA LOGARITMICA (1024 inserciones secuenciales)" << endl;
    AVL<AscendingAVLTrait<T1>> big;
    for (T1 i = 1; i <= 1024; ++i) big.insert(i, i);
    cout << "  size()   : " << big.size()   << " (esperado 1024)" << endl;
    cout << "  height() : " << big.height() << " (~ log2(1024)=10)" << endl;

    cout << "\n4. ITERADORES HEREDADOS (inorder, preorder, postorder)" << endl;
    AVL<DescendingAVLTrait<T1>> t;
    for (T1 v : {5, 3, 7, 1, 4, 6, 8}) t.insert(v, v * 10);
    cout << "  Inorder    fwd: ";
    for (auto it = t.begin();      it != t.end();      ++it) cout << *it << " ";
    cout << "\n  Preorder   fwd: ";
    for (auto it = t.pre_begin();  it != t.pre_end();  ++it) cout << *it << " ";
    cout << "\n  Postorder  fwd: ";
    for (auto it = t.post_begin(); it != t.post_end();++it) cout << *it << " ";
    cout << endl;

    cout << "\n5. CONCURRENCIA (5 hilos x 1000 inserts)" << endl;
    AVL<AscendingAVLTrait<T1>> conc;
    auto worker = [&conc](T1 id) {
        for (T1 i = 0; i < 1000; ++i) conc.insert(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  size() : " << conc.size()   << " (esperado 5000)" << endl;
    cout << "  height(): " << conc.height() << " (acotado por ~1.44*log2(5000) ~ 18)" << endl;
    cout << "  ESTADO: " << (conc.size() == 5000 ? "[EXITO]" : "[FALLO]") << endl;
}
