#include <iostream>
#include "BinaryTree.h"
#include "traits.h"
#include "../types.h"

using namespace std;

void DemoBinaryTreeAscending() {
    cout << "\nBinary Tree - Ascending Order" << endl;
    
    BinaryTree<AscendingTrait<BTNode<T1>>> bt;
    
    cout << "1. Insertando 7 elementos: 50, 30, 70, 20, 40, 60, 80" << endl;
    bt.insert(50, 10);
    bt.insert(30, 20);
    bt.insert(70, 30);
    bt.insert(20, 40);
    bt.insert(40, 50);
    bt.insert(60, 60);
    bt.insert(80, 70);
    cout << "   Tamaño: " << bt.size() << endl;
    
    cout << "2. Buscando elemento valor=40:" << endl;
    try {
        auto [data, ref] = bt.search(40);
        cout << "Encontrado: (" << data << ", " << ref << ")" << endl;
    } catch (const exception& e) {
        cout << "No encontrado" << endl;
    }
    
    cout << "3. Buscando elemento valor=25:" << endl;
    try {
        auto [data, ref] = bt.search(25);
        cout << "Encontrado: (" << data << ", " << ref << ")" << endl;
    } catch (const exception& e) {
        cout << "No encontrado" << endl;
    }
    
    cout << "4. Verificando si esta balanceado:" << endl;
    if (bt.isBalanced()) {
        cout << "Árbol esta balanceado" << endl;
    } else {
        cout << "Árbol no balanceado" << endl;
    }
}

void DemoBinaryTreeDescending() {
    cout << "\nBinary Tree - Descending Order" << endl;
    
    BinaryTree<DescendingTrait<BTNode<T1>>> bt;
    
    cout << "1. Insertando 7 elementos: 50, 30, 70, 20, 40, 60, 80" << endl;
    bt.insert(50, 10);
    bt.insert(30, 20);
    bt.insert(70, 30);
    bt.insert(20, 40);
    bt.insert(40, 50);
    bt.insert(60, 60);
    bt.insert(80, 70);
    cout << "   Tamaño: " << bt.size() << endl;
    
    cout << "2. Buscando elemento valor=50:" << endl;
    try {
        auto [data, ref] = bt.search(50);
        cout << "   Encontrado: (" << data << ", " << ref << ")" << endl;
    } catch (const exception& e) {
        cout << "   No encontrado" << endl;
    }
}

void BinaryTreeDemo() {
    cout << "BINARY TREE DEMO" << endl;
    DemoBinaryTreeAscending();
    DemoBinaryTreeDescending();

}
