#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

template <typename T>
class DLLNode : public LLNodeBase<T, DLLNode<T>>{
public:
    using Node = DLLNode<T>;
    using value_type = T;

private:
    Node *m_pPrev;

public:
    // Reutilizacion Node en DLL
    DLLNode() : LLNodeBase<T, Node>(), m_pPrev(nullptr) {}
    DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr)
        : LLNodeBase<T, Node>(data, ref, next), m_pPrev(prev) {}

    Node *getPrev() const { return m_pPrev; }
    void setPrev(Node *prev) { m_pPrev = prev; }
    Node *&getPrevRef() { return m_pPrev; }
};

template <typename T>
struct AscendingDLLTrait : BaseTrait<DLLNode<T>, std::less<T>>{
};

template <typename T>
struct DescendingDLLTrait : BaseTrait<DLLNode<T>, std::greater<T>>{
};

template <typename Container>
class DoubleLinkedListBackwardIterator : public general_iterator<Container, DoubleLinkedListBackwardIterator<Container>>{
public:
    using MySelf = DoubleLinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    // Reutilizacion iterator
    MySelf operator++(){
        if(this->m_pNode){
            this->m_pNode = this->m_pNode->getPrev();
        }
        return *this;
    }
};

template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait>{
public:
    using Parent = LinkedList<Trait>;
    using value_type = typename Parent::value_type;
    using Node = typename Parent::Node;
    using backward_iterator = DoubleLinkedListBackwardIterator<DoubleLinkedList<Trait>>;
    friend backward_iterator;

    DoubleLinkedList() {}

    // copy y move 
    DoubleLinkedList(const DoubleLinkedList &other) : Parent() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_comp = other.m_comp;
        for(Node *curr = other.m_pRoot; curr; curr = curr->getNext()){
            push_back(curr->getData(), curr->getRef());
        }
    }

    DoubleLinkedList(DoubleLinkedList &&other) : Parent() {
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail = std::exchange(other.m_tail, nullptr);
        this->m_size = std::exchange(other.m_size, 0);
        this->m_comp = std::move(other.m_comp);
    }

    DoubleLinkedList &operator=(const DoubleLinkedList &other){
        if(this == &other){
            return *this;
        }
        DoubleLinkedList temp(other);
        unique_lock<shared_mutex> lock(this->m_mtx);
        this->clear_unlocked();
        this->m_pRoot = std::exchange(temp.m_pRoot, nullptr);
        this->m_tail = std::exchange(temp.m_tail, nullptr);
        this->m_size = std::exchange(temp.m_size, 0);
        this->m_comp = std::move(temp.m_comp);
        return *this;
    }

    DoubleLinkedList &operator=(DoubleLinkedList &&other){
        if(this == &other){
            return *this;
        }
        unique_lock<shared_mutex> lock(this->m_mtx);
        unique_lock<shared_mutex> otherLock(other.m_mtx);
        this->clear_unlocked();
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail = std::exchange(other.m_tail, nullptr);
        this->m_size = std::exchange(other.m_size, 0);
        this->m_comp = std::move(other.m_comp);
        return *this;
    }

    typename Parent::forward_iterator begin(){ return Parent::begin(); }
    typename Parent::forward_iterator end(){ return Parent::end(); }

    backward_iterator rbegin(){ return backward_iterator(this, this->m_tail); }
    backward_iterator rend(){ return backward_iterator(this, nullptr); }

    // adaptacion insercion 
    void insert(const value_type &value, Ref ref) override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *prev = nullptr;
        Node *curr = this->m_pRoot;
        while(curr && !this->m_comp(value, curr->getDataRef())){
            prev = curr;
            curr = curr->getNext();
        }
        Node *node = new Node(value, ref, curr, prev);
        if(prev){
            prev->setNext(node);
        }else{
            this->m_pRoot = node;
        }
        if(curr){
            curr->setPrev(node);
        }else{
            this->m_tail = node;
        }
        ++this->m_size;
    }

    void push_front(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *node = new Node(value, ref, this->m_pRoot, nullptr);
        if(this->m_pRoot){
            this->m_pRoot->setPrev(node);
        }else{
            this->m_tail = node;
        }
        this->m_pRoot = node;
        ++this->m_size;
    }

    void push_back(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *node = new Node(value, ref, nullptr, this->m_tail);
        if(this->m_tail){
            this->m_tail->setNext(node);
        }else{
            this->m_pRoot = node;
        }
        this->m_tail = node;
        ++this->m_size;
    }

    tuple<value_type, Ref> pop_front() override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot){
            throw runtime_error("La lista esta vacia");
        }
        Node *temp = this->m_pRoot;
        auto result = make_tuple(temp->getData(), temp->getRef());
        this->m_pRoot = temp->getNext();
        if(this->m_pRoot){
            this->m_pRoot->setPrev(nullptr);
        }else{
            this->m_tail = nullptr;
        }
        delete temp;
        --this->m_size;
        return result;
    }

    tuple<value_type, Ref> pop_back() override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_tail){
            throw runtime_error("La lista esta vacia");
        }
        Node *temp = this->m_tail;
        auto result = make_tuple(temp->getData(), temp->getRef());
        this->m_tail = temp->getPrev();
        if(this->m_tail){
            this->m_tail->setNext(nullptr);
        }else{
            this->m_pRoot = nullptr;
        }
        delete temp;
        --this->m_size;
        return result;
    }

    friend istream &operator>>(istream &is, DoubleLinkedList &list){
        char ch;
        if(!(is >> ch) || ch != '['){
            is.clear(ios_base::failbit);
            return is;
        }

        DoubleLinkedList temp;
        value_type val;
        Ref ref;
        char comma, parenClose;
        while(is >> ch && ch != ']'){
            if(ch == '('){
                if(is >> val >> comma >> ref >> parenClose){
                    if(comma == ',' && parenClose == ')'){
                        temp.push_back(val, ref);
                    }
                }
            }
        }
        list = std::move(temp);
        return is;
    }
};

#endif // __DOUBLELINKEDLIST_H__
