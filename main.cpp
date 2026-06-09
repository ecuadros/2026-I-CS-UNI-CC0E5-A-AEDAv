#include "containers/vector.h"
#include "containers/linkedlist.h"
#include "containers/BinaryTree.h"
#include "containers/avl.h"
#include "containers/heap.h"
#include "containers/hash_table.h"
// g++ -std=c++2b main.cpp containers/vector.cpp -o main
void ListsDemo();
void BinaryTreeDemo();
void AVLDemo();
void HeapDemo();
void DemoHashTable();

int main(){
    ListsDemo();
    BinaryTreeDemo();
    AVLDemo();
    HeapDemo();
    DemoHashTable();
    return 0;
}