#include "containers/vector.h"
#include "containers/linkedlist.h"
// g++ -std=c++2b main.cpp containers/vector.cpp -o main
void ListsDemo();
void BinaryTreeDemo();
void AVLDemo();
void DemoMinHeap();
void DemoMaxHeap();
void DemoHashTable();
int main(){
    // DemoVector();
    //DemoConcurrentVector();
    ListsDemo();
    BinaryTreeDemo();
    AVLDemo();
    DemoMinHeap();
    DemoMaxHeap();
    DemoHashTable();
    return 0;
}
