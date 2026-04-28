
#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

// TODO Los iteradores ahora son forward y backward
// Crear 2 nuevos i
template <typename Container>
class DLLForwardIterator : public general_iterator<Container, DLLForwardIterator<Container>>{
public:
    using MySelf = DLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
        
    // TODO operator ++
    MySelf& operator++() {
        if (this->m_pNode)
        this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

template <typename Container>
class DLLBackwardIterator : public general_iterator<Container, DLLBackwardIterator<Container>>{
public:
    using MySelf = DLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    // TODO operator ++
    MySelf& operator++() {
        if (this->m_pNode)
        this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

// TODO DoubleLinkedListNode
template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>>{
    using Node = DLLNode<T>;
private:
    Node *m_pPrev;
public:
    DLLNode() : LLNode<T, DLLNode<T>>(), m_pPrev(nullptr) {}
    DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr) : LLNode<T, DLLNode<T>>(data, ref, next), m_pPrev(prev) {}
    
    // TODO destructor
    ~DLLNode() {}

    Node*  getPrev() const     { return m_pPrev; }
    void   setPrev(Node *prev) { m_pPrev = prev; }
    Node*& getPrevRef()        { return m_pPrev; }

};

// TODO AscendingDLLTrait
template <typename T>
struct AscendingDLLTrait : BaseTrait<T, less<T>>{
    using Node = DLLNode<T>;
};

// TODO DescendingDLLTrait
template <typename T>
struct DescendingDLLTrait : BaseTrait<T, greater<T>>{
    using Node = DLLNode<T>;
};

// TODO DoubleLinkedList
template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait>{
public:
    using value_type        = typename Trait::value_type;
    using Node              = typename Trait::Node;
    using MySelf            = DoubleLinkedList<Trait>;
    using forward_iterator  = DLLForwardIterator <MySelf>;
    using backward_iterator = DLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

private:
    //TODO Internal insert
    void internal_insert(Node *&current, Node *pPrevNode,const value_type &value, Ref ref){
        if (!current || this->m_comp(value, current->getDataRef())){
            Node *newNode = new Node(value, ref, current, pPrevNode);
            if (current)
                current->setPrev(newNode);
            current = newNode;
            this->m_size++;
            if (newNode->getNext() == nullptr)
                this->m_tail = newNode;
            return;
        }
        internal_insert(current->getNextRef(), current, value, ref);
    }

public:
    //constructores
    DoubleLinkedList() : LinkedList<Trait>() {}
    // TODO: Copy constructor
    DoubleLinkedList(const DoubleLinkedList &other): LinkedList<Trait>(){
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node *c = other.m_pRoot; c != nullptr; c = c->getNext())
            this->push_back(c->getData(), c->getRef());
    }

    // TODO: Move constructor
    DoubleLinkedList(DoubleLinkedList &&other): LinkedList<Trait>(){
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail  = std::exchange(other.m_tail,  nullptr);
        this->m_size  = std::exchange(other.m_size,  0);
    }
    
    // TODO copy assignment
    DoubleLinkedList &operator=(const DoubleLinkedList &other){
        if (this != &other){
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            for (Node *c = other.m_pRoot; c != nullptr; c = c->getNext())
                this->push_back(c->getData(), c->getRef());
        }
        return *this;
    }

    // TODO move assignment
    DoubleLinkedList &operator=(DoubleLinkedList &&other){
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
    virtual ~DoubleLinkedList() { clear(); }
    void clear(){
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *current = this->m_pRoot;
        while (current) {
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
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref, this->m_pRoot, nullptr);

        if (!this->m_pRoot)
            this->m_tail = newNode;
        else
            this->m_pRoot->setPrev(newNode);
        
        this->m_pRoot = newNode;
        this->m_size++;
    }

    // TODO pop front devuelve tupla (value, ref)
    std::tuple<value_type, Ref> pop_front() override{
        unique_lock<shared_mutex> lock(this->m_mtx);

        if(!this->m_pRoot)
            throw runtime_error("La lista esta vacia");

        Node *temp = this->m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        this->m_pRoot = temp->getNext();

        if(!this->m_pRoot)
            this->m_tail = nullptr;
        else
            this->m_pRoot->setPrev(nullptr);

        delete temp;
        this->m_size--;
        return result;
    }

    // TODO push back
    void push_back(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref, nullptr, this->m_tail);

        if (!this->m_tail)
            this->m_pRoot = newNode;
        else
            this->m_tail->setNext(newNode);

        this->m_tail = newNode;
        this->m_size++;
    }

    // TODO pop back devuelve tupla (value, ref)
    std::tuple<value_type, Ref> pop_back() override{
        unique_lock<shared_mutex> lock(this->m_mtx);

        if (!this->m_pRoot) 
            throw runtime_error("La lista esta vacia");

        Node *temp   = this->m_tail;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        this->m_tail       = temp->getPrev();

        if (!this->m_tail) 
            this->m_pRoot = nullptr;
        else
            this->m_tail->setNext(nullptr);
        delete temp;
        this->m_size--;
        return result;
    }
    
    // TODO insert
    void insert(const value_type &value, Ref ref) override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        internal_insert(this->m_pRoot, nullptr, value, ref);
        if (this->m_size == 1) this->m_tail = this->m_pRoot;
    }

    // TODO: operator[]
    value_type &operator[](size_t index) override{
        shared_lock<shared_mutex> lock(this->m_mtx);
        if (index >= this->m_size)
            throw out_of_range("Indice fuera de rango");
        
        Node *current = this->m_pRoot;
        for (size_t i = 0; i < index; ++i)
            current = current->getNext();
        return current->getDataRef();
    }

    // TODO: size
    size_t size() const override{
        shared_lock<shared_mutex> lock(this->m_mtx);
        return this->m_size;
    }

    // TODO: iteradores
    forward_iterator  begin()  { return forward_iterator (this, this->m_pRoot); }
    forward_iterator  end()    { return forward_iterator (this, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, this->m_tail);  }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    // TODO: foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        for (auto &item : *this)
            func(item, std::forward<Args>(args)...);
    }

    // TODO: reverse foreach
    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it, std::forward<Args>(args)...);
    }

    // TODO: operator<<
    friend ostream &operator<<(ostream &os, const DoubleLinkedList &list){
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node *current = list.m_pRoot;
        while (current){
            os << "(" << current->getData() << "," << current->getRef() << ")";

            if (current->getNext())
                os << ",";

            current = current->getNext();
        }
        os << "]";
        return os;
    }

    // TODO: operator>>
    friend istream &operator>>(istream &is, DoubleLinkedList &list){
        char ch;

        if (!(is >> ch) || ch != '[')
            throw runtime_error("Formato de entrada incorrecto, se esperaba '['");

        value_type value;
        Ref ref;
        char comma, parenClose;
        while (is >> ch && ch != ']'){
            if (ch == '(')
                if (is >> value >> comma >> ref >> parenClose)
                    if (comma == ',' && parenClose == ')')
                        list.insert(value, ref);
        }
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }

};

#endif