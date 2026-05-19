#include <iostream>
#include <fstream>
#include <thread>
#include"../types.h"
#include"binarytree.h"
using namespace std;

using AscTree = BinaryTree<AscendingTrait<BinaryTreeNode<T1>>>;

void BinaryTreeDemo() {
    cout<<"prueba binarytree"<<endl;

    AscTree t;
    t.insert(5,50); t.insert(3,30); t.insert(7,70);
    t.insert(1,10); t.insert(4,40); t.insert(6,60); t.insert(8,80);

    //t9 toString 
    cout<<"\nToString"<<endl;
    cout<<t.toString()<<endl;
    cout<<endl;

    //t10 operator<<
    cout<<"\noperator<<(consola y archivo)"<<endl;
    {ofstream os("tree.txt"); os<<t; }
    cout<<endl;

    //t11 operator>> 
    cout<<"\noperator>>"<<endl;
    AscTree t2;
    { ifstream is("tree.txt"); is >> t2; }
    cout<<"escrito:"<<t<<endl;
    cout<<"leido:  "<<t2<<endl;
    cout<<endl;

    //t3 copy constructor 
    cout<<"\nCopy constructor"<<endl;
    AscTree tCopy(t);
    cout<<"original:"<<t<<endl;
    cout<<"copia:   "<<tCopy<<endl;
    cout<<endl;

    //t4 move constructor 
    cout<<"\nMove constructor"<<endl;
    AscTree tMove(std::move(tCopy));
    cout<<"movido: "<<tMove<<endl;
    cout<<"vaciado: "<<tCopy<<endl;

    //t5 destructor seguro 
    cout<<"\nDestructor seguro"<<endl;
    { AscTree temp; temp.insert(9,90); temp.insert(2,20); }
    cout<<"destructor OK"<<endl;

    //inorder
    cout<<"\nInorder"<<endl;
    //t6 forward inorder 
    cout<<"fwd:";
    for (auto& val : t.inorder()) cout<<val<<" ";
    cout<<endl;
    //t7 backward inorder
    cout<<"bwd:";
    for (auto it = t.inorder().rbegin(); it != t.inorder().rend(); ++it)
        cout<<*it<<" ";
    cout<<endl;

    //t8 foreach nativo inorder
    cout<<"\nforeach nativo"<<endl;
    cout<<"foreach nativo:";
    for (auto& val : t) cout<<val<<" ";
    cout<<endl;

    //preorder 
    cout<<"\npreorder"<<endl;
    //t13 forward preorder
    cout<<"fwd:";
    t.preorder().forEach([](T1& v){ cout<<v<<" "; });
    cout<<endl;
    //t14 backward preorder
    cout<<"bwd:";
    t.preorder().rForEach([](T1& v){ cout<<v<<" "; });
    cout<<endl;

    //postorder 
    cout<<"\npostorder"<<endl;
    //t15 forward postorder
    cout<<"fwd:";
    t.postorder().forEach([](T1& v){ cout<<v<<" "; });
    cout<<endl;
    //t16 backward postorder
    cout<<"bwd:";
    t.postorder().rForEach([](T1& v){ cout<<v<<" "; });
    cout<<endl;

    //t17contains
    cout<<"\nContains"<<endl;
    cout<<"contains(4): "<<(t.contains(4) ? "true" : "false")<<endl;
    cout<<"contains(9): "<<(t.contains(9) ? "true" : "false")<<endl;

    //t12 concurrencia 
    cout<<"\nConcurrencia"<<endl;
    AscTree tConc;
    auto worker = [&tConc](int id) {
        for (int i = 0; i < 200; i++) tConc.insert(i * id, id);
    };
    thread th1(worker,1), th2(worker,2), th3(worker,3),
           th4(worker,4), th5(worker,5);
    th1.join(); th2.join(); th3.join(); th4.join(); th5.join();
    cout<<"esperado 1000: "<<tConc.size()<<endl;
    cout<<"\nfin de pruebas"<<endl;
}