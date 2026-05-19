#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"
template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>>{
    DLLNode *m_prev;
public:
    using value_type = T;
    DLLNode() : LLNode<T, DLLNode<T>>(), m_prev(nullptr) {}
    DLLNode(T data, Ref ref, DLLNode *next = nullptr, DLLNode *prev = nullptr) : LLNode<T, DLLNode<T>>(data, ref, next), m_prev(prev) {}
    DLLNode* getPrev() const     { return m_prev; }
    void   setPrev(DLLNode *prev) { m_prev = prev;    }
};

template <typename Container>
class DLLBackwardIterator: public general_iterator<Container, DLLBackwardIterator<Container>> {
public:
    using MySelf = DLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    using Parent::Parent;
    MySelf &operator++() {
        if (this->m_pNode)
            this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

// ─── DoubleLinkedList ─────────────────────────────────────────────────────────
template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node= typename Trait::Node;
    using MySelf= DoubleLinkedList<Trait>;
    using backward_iterator= DLLBackwardIterator<MySelf>;
    friend backward_iterator;

    DoubleLinkedList() : LinkedList<Trait>() {}

    //push back
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) { this->m_pRoot = this->m_tail = newNode; }
        else {
            this->m_tail->setNext(newNode);
            newNode->setPrev(this->m_tail);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    //push front
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref, this->m_pRoot);
        if (this->m_size == 0) { this->m_tail = newNode; }
        else                   { this->m_pRoot->setPrev(newNode); }
        this->m_pRoot = newNode;
        this->m_size++;
    }

    //pop front
    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node *temp   = this->m_pRoot;
        auto  result = make_tuple(temp->getData(), temp->getRef());
        this->m_pRoot = temp->getNext();
        if (this->m_pRoot) { this->m_pRoot->setPrev(nullptr); }
        else               { this->m_tail = nullptr; }
        delete temp;
        this->m_size--;
        return result;
    }

    //pop back
    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node *temp   = this->m_tail;
        auto  result = make_tuple(temp->getData(), temp->getRef());
        this->m_tail = temp->getPrev();
        if (this->m_tail) { this->m_tail->setNext(nullptr); }
        else              { this->m_pRoot = nullptr; }
        delete temp;
        this->m_size--;
        return result;
    }

    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) { this->m_pRoot = this->m_tail = newNode; }
        else if (this->m_comp(value, this->m_pRoot->getDataRef())) {
            newNode->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(newNode);
            this->m_pRoot = newNode;
        } else {
            Node *act = this->m_pRoot;
            while (act->getNext() && !this->m_comp(value, act->getNext()->getDataRef()))
                act = act->getNext();
            newNode->setNext(act->getNext());
            if (act->getNext()) { act->getNext()->setPrev(newNode); }
            else                { this->m_tail = newNode; }
            act->setNext(newNode);
            newNode->setPrev(act);
        }
        this->m_size++;
    }

    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it, forward<Args>(args)...);
    }
};

#endif // __DOUBLELINKEDLIST_H__