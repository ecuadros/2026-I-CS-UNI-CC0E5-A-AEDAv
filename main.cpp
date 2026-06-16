#include "containers/vector.h"
#include "containers/heap.h"
// g++ -std=c++2b main.cpp containers/vector.cpp -o main
void HeapDemo();
void DemoHashAVL();

int main(){
    // DemoVector();
    //DemoConcurrentVector();
    //ListsDemo();
    HeapDemo();
    DemoHashAVL();

    return 0;
}