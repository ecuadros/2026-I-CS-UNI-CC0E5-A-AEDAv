#include <iostream>
#include "containers/vector.h"
#include "containers/linkedlist.h"
#include "containers/doublelinkedlist.h"
#include "containers/circularlinkedlist.h"
#include "containers/circulardoublelinkedlist.h"
#include "containers/BinaryTree.h"
#include "containers/avl.h"

void ListsDemo();
void TreeDemo();
void AVLMapDemo(std::ostream&);
void HashDemo(std::ostream&);
void DemoMinHeap();
void DemoMaxHeap();
void BTreeDemo(std::ostream&);

int main(){
    ListsDemo();
    TreeDemo();
    AVLMapDemo(std::cout);
    HashDemo(std::cout);
    DemoMinHeap();
    DemoMaxHeap();
    BTreeDemo(std::cout);
    return 0;
}
