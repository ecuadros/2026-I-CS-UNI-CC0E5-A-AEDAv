#include "containers/vector.h"
#include "containers/linkedlist.h"
#include "containers/BinaryTree.h"
#include "containers/avl.h"
#include "containers/heap.h"
// g++ -std=c++2b main.cpp containers/vector.cpp -o main
void ListsDemo();
void BinaryTreeDemo();
void AVLDemo();
void HeapDemo();

int main(){
    ListsDemo();
    BinaryTreeDemo();
    AVLDemo();
    HeapDemo();
    return 0;
}