#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <shared_mutex> // shared_mutex
#include "general_iterator.h"
#include "util.h"
#include <mutex>
#include "../types.h"
using namespace std;

// Forward iterator
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
    public:
        using MySelf = LinkedListForwardIterator<Container>;
        using Parent = general_iterator<Container, MySelf>;
        using Parent::Parent;
        // TODO: Completar el operator++
        //t4
        MySelf operator++() {// increment
            if (this->m_pNode) {// Check if the current node is not null
                this->m_pNode=this->m_pNode->getNext();// Move to the next node
            }
            return *this;// Return the updated iterator
        }
};

// Linked List Node
template <typename T>
class LLNode{
    using Node = LLNode<T>;//Node es un alias de LLNode<T>
private:
    T   m_data;
    Node *m_next;
public:
    LLNode() : m_data(T()), m_next(nullptr) {}
    LLNode(T data) : m_data(data), m_next(nullptr) {}
    LLNode(T data, Node *next) : m_data(data), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Node*  getNext() const { return m_next; }
    Node*& getNextRef()    { return m_next; }
    void   setNext(Node *next) { m_next = next; }
};

template <typename T>
struct AscendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = greater<T>;
};

template <typename Trait>
class LinkedList{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = LinkedList<Trait>;//Creamos LinkedList como un tipo generico Trait.
    // Es un tipo ascending o descending.

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    // friend forward_iterator;

private:
    Node *m_pRoot = nullptr;//Puntero al primer nodo de la lista
    Node *m_tail = nullptr;//Puntero al ultimo nodo de la lista
    size_t m_size = 0;//Cantidad de nodos en la lista
    Comp   m_comp;
    mutable shared_mutex m_mtx;
public:
    LinkedList() {}

    //Copy constructor
    LinkedList(const LinkedList &other)
        : m_pRoot(nullptr), m_tail(nullptr), m_size(0)//De manera explicita decimos que los valores iniciales m:pRoot,m_tail sean punteros nulos
    {
        shared_lock<shared_mutex> lock(other.m_mtx); //solo protegemos el source
        Node* currOther = other.m_pRoot;
        Node* prevNew = nullptr;
        while (currOther) {
            // Crear copia del nodo
            Node* newNode = new Node(currOther->getData(), nullptr);

            if (!m_pRoot) {
                m_pRoot = newNode;
            } else {
                prevNew->setNext(newNode);
            }

            prevNew = newNode;
            currOther = currOther->getNext();
            ++m_size;
        }
        m_tail = prevNew;
    }

    //Move constructor: Movemos la lista enlanzada de un objA a un objB , tranferiendo los nodos.
    LinkedList(LinkedList &&other)
        :m_pRoot(nullptr), m_tail(nullptr), m_size(0)
    {
        //t13 Concurrencia
        unique_lock<shared_mutex> lock(other.m_mtx); // protegemos el source

        //Transferimos (movemos) los datos
        m_pRoot = other.m_pRoot;
        m_tail  = other.m_tail;
        m_size  = other.m_size;

        // Dejamos el objeto original vacío
        other.m_pRoot = nullptr;
        other.m_tail  = nullptr;
        other.m_size  = 0;
    }

    // t3:: destructor seguro
    virtual ~LinkedList() {
        unique_lock<shared_mutex> lock(m_mtx); //bloqueo exclusivo

        Node* curr = m_pRoot;
        while (curr) {
            Node* next = curr->getNext();
            delete curr;
            curr = next;
        }

        m_pRoot = nullptr;
        m_tail  = nullptr;
        m_size  = 0;
    }


    LinkedList& operator=(const LinkedList &other){ // Copy assignment operator
    }

    LinkedList& operator=(LinkedList &&other){ // Move assignment operator
    }
    
    virtual void    push_front(value_type value, Ref ref);
    virtual void    pop_front();
    virtual void    push_back(value_type value, Ref ref);
    virtual void    pop_back();

private:
    void    internal_insert(Node* &pParent, const value_type &value, Ref ref);


    virtual void    insert(const value_type &value, Ref ref);
    
    virtual value_type& operator[](size_t index);
    virtual size_t  size() const;

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    // Agregar Foreach
    //T12   : Foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }
    
    //T11: Sobrecarga del operador de insercion para imprimir la lista
    friend ostream& operator<<(ostream& os, const LinkedList<Trait>& list) {
        shared_lock<shared_mutex> lock(list.m_mtx); // solo lectura

        os << "[";

        typename LinkedList<Trait>::Node* curr = list.m_pRoot;

        while (curr) {
            os << curr->getData();

            if (curr->getNext()) {
                os << ", ";
            }

            curr = curr->getNext();
        }

        os << "]";

        return os;
    }



};

template <typename T>
void LinkedList<T>::internal_insert(Node* &pPrev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        pPrev = new Node(value, ref, pPrev);
        m_size++;
        if(pPrev->getNext() == nullptr){
            m_tail = pPrev;}
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename T>
void LinkedList<T>::insert(const value_type &value, Ref ref){
    internal_insert(m_pRoot, value, ref);
}

template <typename T>
void LinkedList<T>::push_back(value_type value, Ref ref) {//Obtiene el valor a agregar y su referencia
    unique_lock<shared_mutex> lock(m_mtx); //  bloqueo de escritura

    // Crear nuevo nodo (último → next = nullptr)
    Node* newNode = new Node(value, nullptr);

    // Caso 1: lista vacía
    if (!m_pRoot) {
        m_pRoot = newNode;
        m_tail  = newNode;
    }
    else {
        // Caso 2: lista con elementos
        m_tail->setNext(newNode);
        m_tail = newNode;
    }

    ++m_size;
}


template <typename Trait>
void LinkedList<Trait>::pop_back() {
    unique_lock<shared_mutex> lock(m_mtx); // escritura

    // Caso 1: lista vacía
    if (!m_pRoot) return;

    // Caso 2: un solo nodo
    if (m_pRoot == m_tail) {
        delete m_pRoot;
        m_pRoot = nullptr;
        m_tail  = nullptr;
        m_size  = 0;
        return;
    }

    // Caso 3: más de un nodo → buscar el penúltimo
    Node* curr = m_pRoot;

    while (curr->getNext() != m_tail) {
        curr = curr->getNext();
    }

    // curr ahora es el penúltimo
    delete m_tail;
    m_tail = curr;
    m_tail->setNext(nullptr);

    --m_size;
}

template <typename Trait>
void LinkedList<Trait>::push_front(value_type value, Ref ref) {
    unique_lock<shared_mutex> lock(m_mtx); // escritura

    // Crear nuevo nodo que apunte al actual root
    Node* newNode = new Node(value, m_pRoot);

    // Actualizar root
    m_pRoot = newNode;

    // Si la lista estaba vacía, también actualizamos tail
    if (!m_tail) {
        m_tail = newNode;
    }

    ++m_size;
}

template <typename Trait>
void LinkedList<Trait>::pop_front() {
    std::unique_lock<std::shared_mutex> lock(m_mtx); // escritura

    // Caso 1: lista vacía
    if (!m_pRoot) return;

    // Guardar el nodo actual
    Node* temp = m_pRoot;

    // Mover el root al siguiente nodo
    m_pRoot = m_pRoot->getNext();

    // Eliminar el nodo anterior
    delete temp;

    --m_size;

    // Caso especial: la lista quedó vacía
    if (!m_pRoot) {
        m_tail = nullptr;
    }
}

template <typename Trait>
typename LinkedList<Trait>::value_type&
LinkedList<Trait>::operator[](size_t index) {
    shared_lock<shared_mutex> lock(m_mtx); // solo lectura

    if (index >= m_size) {
        throw out_of_range("Indice fuera de rango");
    }

    Node* curr = m_pRoot;
    size_t i = 0;

    while (i < index) {
        curr = curr->getNext();
        ++i;
    }

    return curr->getDataRef(); // referencia para poder modificar
}

template <typename Trait>
istream& operator>>(istream& is, LinkedList<Trait>& list) {
    size_t n;
    is >> n; // cantidad de elementos

    for (size_t i = 0; i < n; ++i) {
        typename LinkedList<Trait>::value_type value;
        is >> value;

        list.push_back(value, Ref()); // o el ref que uses
    }

    return is;
}







#endif // __LINKEDLIST_H__