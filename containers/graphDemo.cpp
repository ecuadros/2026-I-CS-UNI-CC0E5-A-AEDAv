#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "graph.h"
#include "../types.h"

using namespace std;

// Imprime una lista de ids de nodo
static void printIds(const vector<T1>& ids){
    cout<<"[";
    for(TSize i = 0; i < ids.size(); ++i){
        if(i > 0) cout << ", ";
        cout << ids[i];
    }
    cout << "]";
}

// Pruebas para grafos
template <typename GraphType>
void graphTests(GraphType& g){
    // Agregar nodos con el operator >>
    g.addNode(1, 10);
    g.addNode(2, 20);
    stringstream ssNodes("(3,30);(4,40);(5,50);(6,60)");
    ssNodes >> g;
    stringstream ssEdges("(1,2,9);(2,3,6);(3,6,5);(2,4,2);(2,5,6);(4,6,8)");
    ssEdges >> g;
    g.addEdge(4,5,12);

    // Contadores no nodos y aristas
    cout<<"Nodos: "<<g.nodeCount()<<endl;
    cout<<"Aristas: "<<g.edgeCount()<<"\n\n";

    // Impresion del grafo 
    cout<<"=== Impresion del grafo ===\n"<<g<<"\n\n";

    // Recorrer nodos (foreach nativo)
    cout<<"=== Nodos(ForEach nativo) ===\n";
    for(auto& node : g.nodes())
        cout<<node<<"\n";
    cout<<"\n";

    // Recorrer nodos con iterador constante
    cout<<"=== Nodos ===\n";
    auto nodes_iterable = g.nodes();
    for(auto it = nodes_iterable.cbegin(); it != nodes_iterable.cend(); ++it)
        cout << *it << "\n";
    cout<<"\n";

    // ForEach propio
    cout<<"=== Nodos (ForEach propio) ===\n";
    g.ForEach([](auto& node){ cout << node << "\n"; });
    cout<<"\n";

    // Recorrer aristas (foreach nativo)
    cout<<"=== Aristas (ForEach nativo) ===\n";
    for(auto& edge : g.edges())
        cout<<edge<<"\n";
    cout<<"\n";

    // Recorrer aristas con iterador constante
    cout<<"=== Aristas ===\n";
    auto edges_iterable = g.edges();
    for(auto it = edges_iterable.cbegin(); it != edges_iterable.cend(); ++it)
        cout << *it << "\n";
    cout<<"\n";

    // Nodos adyacentes
    cout<<"Adyacentes de nodo 2:"<<"\n";
    printIds(g.getAdjacentNodes(2));
    cout<<"\n";

    cout<<"Adyacentes de nodo 4:"<<"\n";
    printIds(g.getAdjacentNodes(4));
    cout<<"\n";

    // Busquedas por id
    if(auto* n = g.findNode(2))
        cout<<"Encontrar nodo 2: "<<*n<<"\n";
    if(auto* e = g.findEdge(0))
        cout<<"Encontrar arista 0: "<<*e<<"\n";
    if(g.findNode(127) == nullptr)
        cout<<"Encontrar nodo 99: no existe\n"s;
    cout<<"\n";

    // Constructores de copia y movimiento
    GraphType copy = g;                         // constructor de copia
    GraphType moved = move(copy);               // constructor de movimiento
    cout<<"Grafo movido: nodos="<<moved.nodeCount()
        <<" | aristas="<<moved.edgeCount() << "\n\n";

    // Eliminar arista y nodo
    g.removeEdge(0);                            // quita la arista con id 0
    g.removeNode(5);                            // quita el nodo 5 y sus aristas
    cout<<"Tras remover arista 0 y nodo 5: nodos="
        <<g.nodeCount() << " | aristas="<<g.edgeCount()<<"\n";    
    cout<<"Adyacentes de nodo 2 ahora: "<<"\n";
    printIds(g.getAdjacentNodes(2));
    cout<<"\n\n";

    // Limpieza
    g.clear();
    cout<<"Despues de la limpieza: nodos="<<g.nodeCount()<<" | aristas="<<g.edgeCount()
        <<" | empty="<<(g.empty() ? "true" : "false")<<"\n";
}



void DemoGraph(){
    cout<<"========== GRAFO NO DIRIGIDO ==========\n\n";
    Graph<UndirectedGraphTrait<MyNodeTrait, MyEdgeTrait>> ndgraph;
    graphTests(ndgraph);

    cout << "\n========== GRAFO DIRIGIDO ==========\n\n";
    Graph<DirectedGraphTrait<MyNodeTrait, MyEdgeTrait>> dgraph;
    graphTests(dgraph);
}
