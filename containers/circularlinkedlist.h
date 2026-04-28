#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

// TODO: Forward Iterator Circular
template <typename Container>
class CLLForwardIterator: public general_iterator<Container, CLLForwardIterator<Container>>{
public:
    using MySelf = CLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start;
    bool  m_started;
public:
    CLLForwardIterator(Container *pContainer, Node *pNode): Parent(pContainer, pNode), m_start(pNode), m_started(false) {}
    CLLForwardIterator(Container *pContainer, Node *pNode, bool sentinel): Parent(pContainer, pNode), m_start(nullptr), m_started(sentinel) {}
    // TODO operator++
    MySelf& operator++(){
        if (this->m_pNode){
            m_started      = true;
            this->m_pNode  = this->m_pNode->getNext();
            if (this->m_pNode == m_start)
                this->m_pNode = nullptr;
        }
        return *this;
    }
};

// TODO: AscendingCLLTrait
template <typename T>
struct AscendingCLLTrait : BaseTrait<T, less<T>> {
    using Node = LLNode<T>;
};

// TODO: DescendingCLLTrait
template <typename T>
struct DescendingCLLTrait : BaseTrait<T, greater<T>> {;
    using Node = LLNode<T>;
};


template <typename Trait>
class CircularLinkedList : public LinkedList<Trait>{
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using Comp             = typename Trait::Comp;
    using MySelf           = CircularLinkedList<Trait>;
    using forward_iterator = CLLForwardIterator<MySelf>;
    friend forward_iterator;
private:
    mutable shared_mutex m_mtx;
    // TODO internal_insert
    void internal_insert(const value_type &value, Ref ref){
        Node *newNode = new Node(value, ref);
        this->m_size++;
        if (!this->m_pRoot){
            newNode->setNext(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
            return;
        }
        // Nuevo value antes del root
        if (this->m_comp(value, this->m_pRoot->getDataRef())){
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
            return;
        }
  
        Node *act = this->m_pRoot;
        while (act->getNext() != this->m_pRoot && !this->m_comp(value, act->getNext()->getDataRef()))
            act = act->getNext();

        newNode->setNext(act->getNext());
        act->setNext(newNode);
        if (act == this->m_tail)
            this->m_tail = newNode;
    }

public:
    //constructores
    CircularLinkedList() : LinkedList<Trait>() {}
    CircularLinkedList(const CircularLinkedList &other): LinkedList<Trait>(){
        shared_lock<shared_mutex> lock(other.m_mtx);
        if (!other.m_pRoot) 
            return;

        Node *current = other.m_pRoot;
        do {
            this->push_back(current->getData(), current->getRef());
            current = current->getNext();
        } while (current != other.m_pRoot);
    }

    // TODO move constructor
    CircularLinkedList(CircularLinkedList &&other): LinkedList<Trait>(){
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail  = std::exchange(other.m_tail,  nullptr);
        this->m_size  = std::exchange(other.m_size,  0);
    }

    // TODO copy assignment
    CircularLinkedList &operator=(const CircularLinkedList &other){
        if (this != &other){
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            if (!other.m_pRoot) 
                return *this;

            Node *current = other.m_pRoot;
            do {
                this->push_back(current->getData(), current->getRef());
                current = current->getNext();
            } while (current != other.m_pRoot);
        }
        return *this;
    }

    // TODO move assignment
    CircularLinkedList &operator=(CircularLinkedList &&other){
        if (this != &other){
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
            this->m_tail  = std::exchange(other.m_tail,  nullptr);
            this->  m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    // TODO destructor seguro
    virtual ~CircularLinkedList() { clear(); }
    void clear(){
        unique_lock<shared_mutex> lock(m_mtx);
        if (this->m_pRoot == nullptr) 
            return;

        this->m_tail->setNext(nullptr);
        Node *current = this->m_pRoot;
        while (current){
            Node *next = current->getNext();
            delete current;
            current = next;
        }
        this->m_pRoot = nullptr;
        this->m_tail  = nullptr;
        this->m_size  = 0;
    }

    // TODO push front
    void push_front(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_pRoot == nullptr){
            newNode->setNext(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        }
        else{
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    // TODO pop front devuelve tupla (value, ref)
    std::tuple<value_type, Ref> pop_front() override{
        unique_lock<shared_mutex> lock(m_mtx);
        if (!this->m_pRoot) 
            throw runtime_error("La lista esta vacia");
        
        Node *temp   = this->m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1){
            this->m_pRoot = nullptr;
            this->m_tail  = nullptr;
        }
        else{
            this->m_pRoot = temp->getNext();
            this->m_tail->setNext(this->m_pRoot); // reparar cierre circular
        }
        delete temp;
        this->m_size--;
        return result;
    }

    // TODO push back
    void push_back(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_pRoot == nullptr){
            newNode->setNext(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        }
        else{
            newNode->setNext(this->m_pRoot);  // cierre circular
            this->m_tail->setNext(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    // TODO pop back devuelve tupla (value, ref)
    std::tuple<value_type, Ref> pop_back() override{
        unique_lock<shared_mutex> lock(m_mtx);
        if (!this->m_pRoot) 
            throw runtime_error("La lista esta vacia");
        auto result = std::make_tuple(this->m_tail->getData(), this->m_tail->getRef());
        if (this->m_size == 1){
            delete this->m_tail;
            this->m_pRoot = nullptr;
            this->m_tail  = nullptr;
        }
        else{
            Node *act = this->m_pRoot;
            while (act->getNext() != this->m_tail)
                act = act->getNext();
            delete this->m_tail;
            this->m_tail = act;
            this->m_tail->setNext(this->m_pRoot);
        }
        this->m_size--;
        return result;
    }

    // TODO: insert
    void insert(const value_type &value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(value, ref);
    }

    // TODO operator[]
    value_type &operator[](size_t index) override{
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= this->m_size) 
            throw out_of_range("Indice fuera de rango");

        Node *current = this->m_pRoot;
        for (size_t i = 0; i < index; ++i)
            current = current->getNext();
        return current->getDataRef();
    }

    // TODO size
    size_t size() const override{
        shared_lock<shared_mutex> lock(m_mtx);
        return this->m_size;
    }

    // TODO iteradores
    forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr, true); }


    // TODO foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (this->m_size == 0) return;
        for (auto &item : *this)
            func(item, std::forward<Args>(args)...);
    }

    // TODO circular foreach
    template <typename Func, typename... Args>
    void circularForEach(size_t loops, Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (!this->m_pRoot || loops == 0) 
            return;

        Node  *current   = this->m_pRoot;
        for (size_t i = 0; i < this->m_size * loops; ++i){
            func(current->getDataRef(), std::forward<Args>(args)...);
            current = current->getNext();
        }
    }

    // TODO operator<<
    friend ostream &operator<<(ostream &os, const CircularLinkedList &list){
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        if (list.m_pRoot){
            Node *current = list.m_pRoot;
            do {
                os << "(" << current->getData() << "," << current->getRef() << ")";
                current = current->getNext();
                if (current != list.m_pRoot) os << ",";
            } while (current != list.m_pRoot);
        }
        os << "]";
        if (list.m_pRoot)
            os << " ->root(" << list.m_pRoot->getData() << ")";

        return os;
    }

    // TODO operator>>
    friend istream &operator>>(istream &is, CircularLinkedList &list){
        char ch;
        if (!(is >> ch) || ch != '[')
            throw runtime_error("Formato de entrada incorrecto, se esperaba '['");

        value_type val;
        Ref        ref;
        char       comma, parenClose;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> val >> comma >> ref >> parenClose)
                    if (comma == ',' && parenClose == ')')
                        list.insert(val, ref);
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }
};

#endif