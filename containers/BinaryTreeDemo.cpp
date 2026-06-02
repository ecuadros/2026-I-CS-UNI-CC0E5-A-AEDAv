#include <iostream>
#include <string>
#include "BinaryTree.h"
#include "traits.h"
using namespace std;

template <typename Tree, typename Predicate>
void DemoGenericTree(Tree& tree, const string& name, Predicate dfs_pred, const string& desc_pred) {
    cout << "\n--- " << name << " ---" << endl;

    // 1. Inserción 
    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);

    // 2. Comprobar Balanceo 
    cout << "Esta balanceado?: " << (tree.isBalanced() ? "Sí" : "No") << endl;

    // 3. Iteradores en Bucle Nativo
    cout << "Recorrido Inorder (Forward): ";
    for (auto val : tree.inorder()) { cout << val << " "; }
    cout << endl;
    
    cout << "Recorrido Inorder (Backward): ";
    for (auto val : tree.inorder().reversed()) cout << val << " ";
    cout << endl;

    cout << "Recorrido Preorder (Forward): ";
    for (auto val : tree.preorder()) { cout << val << " "; }
    cout << endl;

    cout << "Recorrido Preorder (Backward): ";
    for (auto val : tree.preorder().reversed()) cout << val << " ";
    cout << endl;

    cout << "Recorrido Postorder (Forward): ";
    for (auto val : tree.postorder()) { cout << val << " "; }
    cout << endl;

    cout << "Recorrido Postorder (Backward): ";
    for (auto val : tree.postorder().reversed()) cout << val << " ";
    cout << endl;

    // 4. Busqueda DFS Personalizada
    cout << "Buscando (DFS) " << desc_pred << ":" << endl;
    auto resultados = tree.searchAll(dfs_pred);
    
    cout << "Resultados: [ ";
    for (const auto& tupla : resultados) {
        cout << "(" << std::get<0>(tupla) << "," << std::get<1>(tupla) << ") ";
    }
    cout << "]" << endl;
}

void BinaryTreeDemo() {
    BinaryTree<AscendingTrait<BTNode<T1>>> treeAsc;
    BinaryTree<DescendingTrait<BTNode<T1>>> treeDesc;

    DemoGenericTree(
        treeAsc, 
        "Demo arbol binario - ascendente", 
        [](const T1& val) { return val > 35; }, 
        "valores mayores a 35"
    );

    DemoGenericTree(
        treeDesc, 
        "Demo arbol binario - descendente", 
        [](const T1& val) { return val < 45; }, 
        "valores menores a 45"
    );
}