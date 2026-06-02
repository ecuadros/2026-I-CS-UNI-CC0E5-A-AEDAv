#include <iostream>
#include <string>
#include "avl.h"
#include "traits.h"
#include "../types.h"

using namespace std;

template <typename Tree, typename Predicate>
void DemoGenericAVL(Tree& tree, const string& name, Predicate dfs_pred, const string& desc_pred) {
    cout << "\n--- " << name << " ---" << endl;

    // 1. Insercion
    cout << "Insertando secuencia: 10, 20, 30, 40, 50, 25..." << endl;
    tree.insert(10, 1);
    tree.insert(20, 2);
    tree.insert(30, 3);
    tree.insert(40, 4);
    tree.insert(50, 5);
    tree.insert(25, 6);

    // 2. Comprobar Propiedades exclusivas del AVL 
    cout << "Esta balanceado?: " << (tree.isBalanced() ? "Sí" : "No") << endl;
    cout << "Altura de la raíz: " << tree.height() << endl;
    cout << "Factor de balance (Raíz): " << tree.balance() << endl;

    // 3. Iteradores en Bucle Nativo
    cout << "Recorrido Inorder: ";
    for (auto val : tree.inorder()) { cout << val << " "; }
    cout << endl;
    
    cout << "Recorrido Preorder: ";
    for (auto val : tree.preorder()) { cout << val << " "; }
    cout << endl;

    // 4. Búsqueda DFS Personalizada
    cout << "Buscando (DFS) " << desc_pred << ":" << endl;
    auto resultados = tree.searchAll(dfs_pred);
    
    cout << "Resultados: [ ";
    for (const auto& tupla : resultados) {
        cout << "(" << std::get<0>(tupla) << "," << std::get<1>(tupla) << ") ";
    }
    cout << "]" << endl;
}

void AVLDemo() {
    // Instanciamos los arboles AVL
    AVLTree<AscendingTrait<AVLNode<T1>>> avlAsc;
    AVLTree<DescendingTrait<AVLNode<T1>>> avlDesc;
    DemoGenericAVL(
        avlAsc, 
        "Demo arbol AVL (ascendente)", 
        [](const T1& val) { return val > 30; }, 
        "valores mayores a 30"
    );

    DemoGenericAVL(
        avlDesc, 
        "Demo arbol AVL (descendente)", 
        [](const T1& val) { return val < 30; }, 
        "valores menores a 30"
    );
}