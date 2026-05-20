#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include "../types.h"
#include "BinaryTree.h"

using namespace std;

// Helper para imprimir un rango de iteradores con un separador.
template <typename It>
void print_range(const string &label, It first, It last){
    cout << label << ": ";
    for(auto it = first; !(it == last); ++it)   
        cout << *it << " ";
    cout << endl;
}


// Forward / Backward iterator (inorder)  y for-each nativo
void TestInorderIterators(){
    cout << "\nInorder forward / backward / for-each nativo" << endl;
    BinaryTree<AscendingBTTrait<T1>> tree;
    int xs[] = {50, 30, 70, 20, 40, 60, 80, 10, 35};
    for(int x : xs) tree.insert(x, x * 10);

    // 4. Forward inorder (debe salir ordenado ascendente)
    print_range("inorder forward ", tree.begin(),  tree.end());
    // 5. Backward inorder (descendente)
    print_range("inorder backward", tree.rbegin(), tree.rend());

    // 6. Bucle nativo for-each (usa begin()/end() = inorder forward)
    cout << "for(auto& x : tree): ";
    for(auto& x : tree) cout << x << " ";
    cout << endl;
}

// Forward / Backward iterator (preorder)
void TestPreorderIterators(){
    cout << "\nPreorder forward / backward" << endl;
    BinaryTree<AscendingBTTrait<T1>> tree;
    int xs[] = {50, 30, 70, 20, 40, 60, 80};
    for(int x : xs) tree.insert(x, x);

    print_range("preorder forward ", tree.pre_begin(),  tree.pre_end());
    print_range("preorder backward", tree.pre_rbegin(), tree.pre_rend());
}

// Forward / Backward iterator (postorder)
void TestPostorderIterators(){
    cout << "\nPostorder forward / backward" << endl;
    BinaryTree<AscendingBTTrait<T1>> tree;
    int xs[] = {50, 30, 70, 20, 40, 60, 80};
    for(int x : xs) tree.insert(x, x);

    print_range("postorder forward ", tree.post_begin(),  tree.post_end());
    print_range("postorder backward", tree.post_rbegin(), tree.post_rend());
}

//  operator<< (incluye persistencia) + operator>>
void TestStreamOperators(){
    cout << "\nTEST: operator<< (persistencia) y operator>>" << endl;

    BinaryTree<AscendingBTTrait<T1>> tree;
    tree.insert(40, 400);
    tree.insert(20, 200);
    tree.insert(60, 600);
    tree.insert(10, 100);
    tree.insert(30, 300);
    cout << "arbol original: " << tree << endl;

    // Persistencia a archivo
    const string fileName = "tree.txt";
    {
        ofstream os(fileName);
        os << tree;
    }
    cout << "guardado en \"" << fileName << "\"" << endl;

    // Lectura desde archivo a un arbol nuevo
    BinaryTree<AscendingBTTrait<T1>> reread;
    {
        ifstream is(fileName);
        is >> reread;
    }
    cout << "releido desde archivo: " << reread << endl;

    // Round-trip via stringstream
    BinaryTree<AscendingBTTrait<T1>> roundtrip;
    stringstream ss;
    ss << tree;
    ss >> roundtrip;
    cout << "round-trip via stringstream: " << roundtrip << endl;
}

// Concurrencia
void TestConcurrency(){
    cout << "\n[9] TEST: Concurrencia" << endl;
    BinaryTree<AscendingBTTrait<T1>> tree;

    const int kThreads = 5;
    const int kPerThread = 1000;

    auto writer = [&tree](int tid){
        for(int i = 0; i<kPerThread; ++i)
            tree.insert(tid * 10000 + i, tid);
    };

    // Lector concurrente: realiza ForEach mientras los escritores trabajan
    auto reader = [&tree](){
        for(int i = 0; i<50; ++i){
            volatile long sum = 0;
            tree.ForEach([&sum](int& v){ sum += v; });
        }
    };

    thread ts[kThreads];
    for(int i = 0; i < kThreads; ++i) ts[i] = thread(writer, i + 1);
    thread tr1(reader), tr2(reader);
    for(int i = 0; i < kThreads; ++i) ts[i].join();
    tr1.join(); tr2.join();

    const size_t expected = kThreads * kPerThread;
    cout << "hilos escritores: " << kThreads
         << "inserciones por hilo: " << kPerThread << endl;
    cout << "size obtenido:  " << tree.size() << "  (esperado " << expected << ")" << endl;
    cout << "ESTADO: " << (tree.size() == expected ? "EXITO (shared_mutex previno race conditions)": "FALLO (corrupcion)") << endl;
}

void TreeDemo(){
    cout<< "------------- DEMO BinaryTree -------------" << endl;
    TestInorderIterators();
    TestPreorderIterators();
    TestPostorderIterators();
    TestStreamOperators();
    TestConcurrency();
}
