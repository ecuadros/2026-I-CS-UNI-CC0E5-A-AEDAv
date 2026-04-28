#ifndef CIRCULAR_DOUBLE_LINKED_LIST_H
#define CIRCULAR_DOUBLE_LINKED_LIST_H

#include "doublelinkedlist.h"
#include <mutex>
#include <shared_mutex>
#include <iostream>
#include "../types.h"

// TODO CDLLForwardIterator
template <typename Container>
class CDLLForwardIterator: public general_iterator<Container, CDLLForwardIterator<Container>>{
public:
    using MySelf = CDLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start;
public:
    CDLLForwardIterator(Container *pContainer, Node *pNode): Parent(pContainer, pNode), m_start(pNode) {}
    CDLLForwardIterator(Container *pContainer, Node *pNode, bool): Parent(pContainer, pNode), m_start(nullptr) {}
    // TODO operator++
    MySelf operator++(){
        if (this->m_pNode){
            this->m_pNode = this->m_pNode->getNext();
            if (this->m_pNode == m_start)
                this->m_pNode = nullptr;
        }
        return *this;
    }
};

// TODO CDLLBackwardIterator
template <typename Container>
class CDLLBackwardIterator: public general_iterator<Container, CDLLBackwardIterator<Container>>{
public:
    using MySelf = CDLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start;
public:
    CDLLBackwardIterator(Container *pContainer, Node *pNode): Parent(pContainer, pNode), m_start(pNode) {}
    CDLLBackwardIterator(Container *pContainer, Node *pNode, bool): Parent(pContainer, pNode), m_start(nullptr) {}
    // TODO operator++
    MySelf operator++(){
        if (this->m_pNode){
            this->m_pNode = this->m_pNode->getPrev();
            if (this->m_pNode == m_start)
                this->m_pNode = nullptr;
        }
        return *this;
    }
};

template <typename T>
class CDLLNode{
private:
    T        m_data;
    Ref      m_ref;
    CDLLNode *m_next;
    CDLLNode *m_prev;
public:
    CDLLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr), m_prev(nullptr) {}
    CDLLNode(T data, Ref ref, CDLLNode *next = nullptr, CDLLNode *prev = nullptr): m_data(data), m_ref(ref), m_next(next), m_prev(prev) {}
    virtual ~CDLLNode() {}
    T         getData()  const     { return m_data; }
    T        &getDataRef()         { return m_data; }
    void      setData(T data)      { m_data = data; }
    Ref       getRef()   const     { return m_ref; }
    void      setRef(Ref ref)      { m_ref = ref; }
    CDLLNode *getNext()  const     { return m_next; }
    CDLLNode *&getNextRef()        { return m_next; }
    void      setNext(CDLLNode *n) { m_next = n; }
    CDLLNode *getPrev()  const     { return m_prev; }
    CDLLNode *&getPrevRef()        { return m_prev; }
    void      setPrev(CDLLNode *p) { m_prev = p; }
};

// TODO AscendingCDLLTrait
template <typename T>
struct AscendingCDLLTrait : BaseTrait<T, less<T>>{
    using Node = CDLLNode<T>;
};

// TODO DescendingCDLLTrait
template <typename T>
struct DescendingCDLLTrait : BaseTrait<T, greater<T>>{
    using Node = CDLLNode<T>;
};

// TODO: CircularDoubleLinkedList
template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = CircularDoubleLinkedList<Trait>;
    using forward_iterator  = CDLLForwardIterator<MySelf>;
    using backward_iterator = CDLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

private:
    void internal_insert(const value_type &value, Ref ref) {
        Node *newNode = new Node(value, ref);
        this->m_size++;

        if (!this->m_pRoot) {
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_pRoot = newNode;
            this->m_tail = newNode;
            return;
        }

        if (this->m_comp(value, this->m_pRoot->getDataRef())) {
            Node *last = this->m_tail;
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(last);
            this->m_pRoot->setPrev(newNode);
            last->setNext(newNode);
            this->m_pRoot = newNode;
            return;
        }

        Node *current = this->m_pRoot;
        while (current->getNext() != this->m_pRoot && !this->m_comp(value, current->getNext()->getDataRef())) {
            current = current->getNext();
        }

        Node *next_node = current->getNext();
        newNode->setNext(next_node);
        newNode->setPrev(current);
        current->setNext(newNode);
        next_node->setPrev(newNode);

        if (current == this->m_tail)
            this->m_tail = newNode;
    }

public:
    //constructores
    CircularDoubleLinkedList() : DoubleLinkedList<Trait>() {}
    // TODO: copy constructor
    CircularDoubleLinkedList(const CircularDoubleLinkedList &other): DoubleLinkedList<Trait>(){
        shared_lock<shared_mutex> lock(other.m_mtx);
        if (!other.m_pRoot) return;
        Node *curr = other.m_pRoot;
        do {
            this->push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        } while (curr != other.m_pRoot);
    }

    // TODO: move constructor
    CircularDoubleLinkedList(CircularDoubleLinkedList &&other): DoubleLinkedList<Trait>(){
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail  = std::exchange(other.m_tail,  nullptr);
        this->m_size  = std::exchange(other.m_size,  0);
    }

    // TODO: copy assignment
    CircularDoubleLinkedList &operator=(const CircularDoubleLinkedList &other){
        if (this != &other){
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            if (!other.m_pRoot) return *this;
            Node *curr = other.m_pRoot;
            do {
                this->push_back(curr->getData(), curr->getRef());
                curr = curr->getNext();
            } while (curr != other.m_pRoot);
        }
        return *this;
    }

    // TODO: move assignment
    CircularDoubleLinkedList &operator=(CircularDoubleLinkedList &&other){
        if (this != &other){
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
            this->m_tail  = std::exchange(other.m_tail,  nullptr);
            this->m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    // TODO destructor seguro
    virtual ~CircularDoubleLinkedList() {
        clear();
    }
    void clear() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) return;

        Node* current = this->m_pRoot;
        Node* next = nullptr;

        this->m_tail->setNext(nullptr);

        while (current != nullptr) {
            next = current->getNext();
            delete current;
            current = next;
        }

        this->m_pRoot = nullptr;
        this->m_tail = nullptr;
        this->m_size = 0;
    }

    // TODO push front
    void push_front(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (!this->m_pRoot){
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        }
        else{
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);
            this->m_pRoot->setPrev(newNode);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    // TODO pop front: devuelve tupla (value, ref)
    std::tuple<value_type, Ref> pop_front() override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node *temp   = this->m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1){
            this->m_pRoot = nullptr;
            this->m_tail  = nullptr;
        }
        else{
            this->m_pRoot = temp->getNext();
            this->m_pRoot->setPrev(this->m_tail);
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    // TODO push back
    void push_back(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (!this->m_pRoot){
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        }
        else{
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);
            this->m_tail->setNext(newNode);
            this->m_pRoot->setPrev(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    // TODO pop back devuelve tupla (value, ref)
    std::tuple<value_type, Ref> pop_back() override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("listavacia");
        Node *temp   = this->m_tail;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1){
            this->m_pRoot = nullptr;
            this->m_tail  = nullptr;
        }
        else{
            this->m_tail = temp->getPrev();
            this->m_tail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_tail);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    // TODO: insert
    void insert(const value_type &value, Ref ref) override {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);
        internal_insert(value, ref);
    }


    // TODO operator[]
    value_type &operator[](size_t index) override{
        shared_lock<shared_mutex> lock(this->m_mtx);
        if (index >= this->m_size) throw out_of_range("fuera de rango");
        Node *current = this->m_pRoot;
        for (size_t i = 0; i < index; ++i)
            current = current->getNext();
        return current->getDataRef();
    }

    // TODO size
    size_t size() const override{
        shared_lock<shared_mutex> lock(this->m_mtx);
        return this->m_size;
    }

    // TODO iteradores
    forward_iterator  begin()  { return forward_iterator (this, this->m_pRoot); }
    forward_iterator  end()    { return forward_iterator (this, nullptr, true); }
    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr, true); }

    // TODO foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        for (auto &item : *this)
            func(item, std::forward<Args>(args)...);
    }

    // TODO reverse foreach
    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it, std::forward<Args>(args)...);
    }

    // TODO circular foreach
    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, int direction, Func func, Args &&...args){
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot || vueltas == 0) return;
        Node  *current   = (direction >= 0) ? this->m_pRoot : this->m_tail;
        size_t pasos = this->m_size * vueltas;
        for (size_t i = 0; i < pasos; ++i){
            func(current->getDataRef(), std::forward<Args>(args)...);
            current = (direction >= 0) ? current->getNext() : current->getPrev();
        }
    }

    // TODO operador<<
    friend ostream& operator<<(ostream& os, const CircularDoubleLinkedList<Trait>& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        if (list.m_pRoot) {
            Node *current = list.m_pRoot;
            do {
                os << "(" << current->getData() << "," << current->getRef() << ")";
                current = current->getNext();
                if (current != list.m_pRoot) os << ",";
            } while (current != list.m_pRoot);
        }
        os << "]";

        if (list.m_pRoot)
            os << " ->root(" << list.m_pRoot->getData()<< "," << list.m_pRoot->getRef() << ")";
        return os;
    }

    // TODO operador>>
    friend istream& operator>>(istream& is, CircularDoubleLinkedList& list) {
        char ch;
        if (!(is >> ch) || ch != '[') 
            throw runtime_error("Formato de entrada incorrecto, se esperaba '['");

        value_type val;
        Ref ref;
        char comma, parenClose;
        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref >> parenClose) {
                    if (comma == ',' && parenClose == ')') {
                        list.insert(val, ref);
                    }
                }
            }
        }
        return is;
    }

};

#endif