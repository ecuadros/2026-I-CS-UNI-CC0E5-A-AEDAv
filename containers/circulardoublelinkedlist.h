#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "circularlinkedlist.h"
#include "doublelinkedlist.h"

// Reutilizacion de iteradores
template <typename Container>
class CDLLBackwardIterator : public general_iterator<Container, CDLLBackwardIterator<Container>> {
public:
    using MySelf = CDLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;

private:
    Node* m_startNode;

public:
    CDLLBackwardIterator(Container *c, Node *node, Node *startNode) 
        : Parent(c, node), m_startNode(startNode) {}

    MySelf& operator++() {
        if (this->m_pNode) {
            Node *prev    = this->m_pNode->getPrev();
            this->m_pNode = (prev == m_startNode) ? nullptr : prev;
        }
        return *this;
    }
};

template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using value_type        = typename Trait::value_type;
    using Node              = typename Trait::Node;
    using MySelf            = CircularDoubleLinkedList<Trait>;
    using forward_iterator  = CLLForwardIterator<MySelf>;
    using backward_iterator = CDLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

    CircularDoubleLinkedList() : DoubleLinkedList<Trait>() {}

    // destructor
    void clear() override {
        if (this->m_tail)  this->m_tail->setNext(nullptr);
        if (this->m_pRoot) this->m_pRoot->setPrev(nullptr);
        LinkedList<Trait>::clear();
    }

    forward_iterator begin() const {
        return forward_iterator(const_cast<MySelf*>(this), this->m_pRoot, this->m_pRoot);
    }
    forward_iterator end() const {
        return forward_iterator(const_cast<MySelf*>(this), nullptr, this->m_pRoot);
    }
    backward_iterator rbegin() {
        return backward_iterator(this, this->m_tail, this->m_tail);
    }
    backward_iterator rend() {
        return backward_iterator(this, nullptr, this->m_tail);
    }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
        } else {
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);
            this->m_tail->setNext(newNode);
            this->m_pRoot->setPrev(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
        } else {
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);
            this->m_pRoot->setPrev(newNode);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node *temp   = this->m_pRoot;
        auto  result = make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            this->m_pRoot = temp->getNext();
            this->m_pRoot->setPrev(this->m_tail);
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node *temp   = this->m_tail;
        auto  result = make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            this->m_tail = temp->getPrev();
            this->m_tail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_tail);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    // internal_insert
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
        } else if (this->m_comp(value, this->m_pRoot->getDataRef())) {
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);
            this->m_pRoot->setPrev(newNode);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        } else {
            Node *act = this->m_pRoot;
            while (act->getNext() != this->m_pRoot &&
                   !this->m_comp(value, act->getNext()->getDataRef()))
                act = act->getNext();
            Node *following = act->getNext();
            newNode->setNext(following);
            newNode->setPrev(act);
            act->setNext(newNode);
            following->setPrev(newNode);
            if (act == this->m_tail) this->m_tail = newNode;
        }
        this->m_size++;
    }

    // circularForEach
    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, int direction, Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot || vueltas == 0) return;
        Node  *act   = (direction >= 0) ? this->m_pRoot : this->m_tail;
        size_t pasos = this->m_size * vueltas;
        for (size_t i = 0; i < pasos; ++i) {
            func(act->getDataRef(), std::forward<Args>(args)...);
            act = (direction >= 0) ? act->getNext() : act->getPrev();
        }
    }
};

#endif // __CIRCULARDOUBLELINKEDLIST_H__