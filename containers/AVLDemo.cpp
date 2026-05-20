#include <iostream>
#include "avl.h"
#include "traits.h"
#include "../types.h"

using namespace std;

void AVLDemo() {
    // AVL Ascendente
    cout << "\nAVL Tree" << endl;
    AVLTree<AscendingTrait<AVLNode<T1>>> avl_asc;
    
    cout << "1. Insertando secuencia con rotaciones: 10, 20, 30, 40, 50, 25" << endl;
    avl_asc.insert(10, 1);
    avl_asc.insert(20, 2);
    avl_asc.insert(30, 3);
    avl_asc.insert(40, 4);
    avl_asc.insert(50, 5);
    avl_asc.insert(25, 6);
    cout << "Tamaño: " << avl_asc.size() << endl;
    cout << "Balanceado? " << (avl_asc.isBalanced() ? "Si" : "No") << endl;

    cout << "2. Recorrido InOrder:" << endl;
    avl_asc.inorder().forEach([](const auto& item) {
        cout << item.first << " ";
    });
    cout << endl;
    
    cout << "3. Recorrido PreOrder:" << endl;
    avl_asc.preorder().forEach([](const auto& item) {
        cout << item.first << " ";
    });

    cout << "4. Copy constructor test:" << endl;
    AVLTree<AscendingTrait<AVLNode<T1>>> avl_copy(avl_asc);
    cout << "Copia creada, tamaño: " << avl_copy.size() << endl;
    cout << "Copia InOrder: ";
    avl_copy.inorder().forEach([](const auto& item) {
        cout << item.first << " ";
    });
    cout << endl;
    
    // AVL Descendente
    AVLTree<DescendingTrait<AVLNode<T1>>> avl_desc;
    
    cout << "1. Insertando: 50, 30, 70, 20, 40, 60, 80" << endl;
    avl_desc.insert(50, 10);
    avl_desc.insert(30, 20);
    avl_desc.insert(70, 30);
    avl_desc.insert(20, 40);
    avl_desc.insert(40, 50);
    avl_desc.insert(60, 60);
    avl_desc.insert(80, 70);
    cout << "Tamaño: " << avl_desc.size() << endl;
    cout << "Balanceado? " << (avl_desc.isBalanced() ? "Si" : "No") << endl;
    
    cout << "2. Buscando elemento 50:" << endl;
    try {
        auto [data, ref] = avl_desc.search(50);
        cout << " Encontrado: data=" << data << ", ref=" << ref << endl;
    } catch (const exception& e) {
        cout << " No encontrado" << endl;
    }
    
    cout << "3. Recorrido InOrder:" << endl;
    avl_desc.inorder().forEach([](const auto& item) {
        cout << item.first << " ";
    });
}
