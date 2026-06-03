#include <iostream>
#include <fstream>
#include <thread>
#include "../types.h"
#include "avl.h"
using namespace std;

using AscAVL = AVL<AscendingTrait<AVLNode<T1>>>;

void AVLDemo() {
    cout<<"-------------------"<<endl;
    cout<<"PRUEBAS AVL"<<endl;

    //caso 1: inser asc
    cout<<"\ncaso 1: insercion ascendente"<<endl;
    AscAVL t;
    for (int i = 1; i <= 7; i++) {
        t.insert(i, i * 10);
        cout<<"  insert("<<i<<") | altura: "<<t.height()
            <<" | balance: "<<t.balance()<<endl;
        t.printTree();
    }

    //caso 2: insercion descendente
    cout<<"\ncaso 2: insercion descendente"<<endl;
    AscAVL t2;
    for (int i = 7; i >= 1; i--) {
        t2.insert(i, i * 10);
        cout<<"  insert("<<i<<") | altura: "<<t2.height()
            <<" | balance: "<<t2.balance()<<endl;
        t2.printTree();
    }

    //caso 3: insercion mixta
    cout<<"\ncaso 3: insercion mixta (LL, RR, LR, RL)"<<endl;
    AscAVL t3;
    for (int v : {5, 3, 7, 1, 4, 6, 8, 2}) {
        t3.insert(v, v * 10);
        cout<<"  insert("<<v<<") | altura: "<<t3.height()
            <<" | balance: "<<t3.balance()<<endl;
        t3.printTree();
    }

    //caso 4: copy constructor
    cout<<"\nCaso 4: copy constructor preserva altura"<<endl;
    AscAVL tCopy(t);
    cout<<"  original altura: "<<t.height()
        <<" | copia altura: "  <<tCopy.height()<<endl;
    tCopy.printTree();

    //caso 5: concurrencia
    cout<<"\nCaso 5: concurrencia"<<endl;
    AscAVL tConc;
    auto worker = [&tConc](int id) {
        for (int i = 0; i < 200; i++) tConc.insert(i * id, id);
    };
    thread th1(worker,1), th2(worker,2), th3(worker,3),
           th4(worker,4), th5(worker,5);
    th1.join(); th2.join(); th3.join(); th4.join(); th5.join();
    cout<<"size (esperado 1000): "<<tConc.size()   <<endl;
    cout<<"balance raiz [-1,1]:      "<<tConc.balance()<<endl;
    cout<<"altura :  "<<tConc.height() <<endl;

    cout<<"\nFIN DE LAS PRUEBAS"<<endl;
}