#include "containers/vector.h"
#include "containers/linkedlist.h"
// g++ -std=c++2b main.cpp containers/vector.cpp -o main
void ListsDemo();
void HeapDemo();
void DemoHashTable();
void DemoBTree23();
void DemoBTree34();
int main(){
    // DemoVector();
    //DemoConcurrentVector();
    ListsDemo();
    HeapDemo();
    DemoHashTable();
    DemoBTree23();
    DemoBTree34();
    return 0;
}