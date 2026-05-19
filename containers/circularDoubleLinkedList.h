#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "doublelinkedlist.h"
#include "circularlinkedlist.h"

template <typename Container>
class CDLLForwardIterator: public circular_iterator<Container, CDLLForwardIterator<Container>> {
public:
    using MySelf = CDLLForwardIterator<Container>;
    using Parent = circular_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    CDLLForwardIterator(Container *c, Node *node, Node *root): Parent(c, node, root) {}
};

template <typename Container>
class CDLLBackwardIterator
    : public circular_iterator<Container, CDLLBackwardIterator<Container>> {
public:
    using MySelf = CDLLBackwardIterator<Container>;
    using Parent = circular_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    CDLLBackwardIterator(Container *c, Node *node, Node *root)
        : Parent(c, node, root) {}
    MySelf &operator++() {
        if (this->m_pNode) {
            Node *prev    = this->m_pNode->getPrev();
            this->m_pNode = (prev == this->m_pRoot) ? nullptr : prev;
        }
        return *this;
    }
};

template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using value_type         = typename Trait::value_type;
    using Node               = typename Trait::Node;
    using MySelf             = CircularDoubleLinkedList<Trait>;
    using forward_iterator   = CDLLForwardIterator<MySelf>;
    using backward_iterator  = CDLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

    CircularDoubleLinkedList() : DoubleLinkedList<Trait>() {}

    forward_iterator cbegin() const {
        return forward_iterator(const_cast<MySelf *>(this), this->m_pRoot, this->m_pRoot);
    }
    forward_iterator cend() const {
        return forward_iterator(const_cast<MySelf *>(this), nullptr, this->m_pRoot);
    }
    backward_iterator crbegin() {
        return backward_iterator(this, this->m_tail, this->m_tail);
    }
    backward_iterator crend() {
        return backward_iterator(this, nullptr, this->m_tail);
    }

    //push back
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode); newNode->setPrev(newNode);
        } else {
            newNode->setNext(this->m_pRoot); newNode->setPrev(this->m_tail);
            this->m_tail->setNext(newNode);  this->m_pRoot->setPrev(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    //push front
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode); newNode->setPrev(newNode);
        } else {
            newNode->setNext(this->m_pRoot); newNode->setPrev(this->m_tail);
            this->m_pRoot->setPrev(newNode); this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    //pop front
    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("Lista vacia");
        Node *temp   = this->m_pRoot;
        auto  result = make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1) { this->m_pRoot = this->m_tail = nullptr; }
        else {
            this->m_pRoot = temp->getNext();
            this->m_pRoot->setPrev(this->m_tail);
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp; this->m_size--;
        return result;
    }

    //pop back
    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("Lista vacia");
        Node *temp   = this->m_tail;
        auto  result = make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1) { this->m_pRoot = this->m_tail = nullptr; }
        else {
            this->m_tail = temp->getPrev();
            this->m_tail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_tail);
        }
        delete temp; this->m_size--;
        return result;
    }

    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode); newNode->setPrev(newNode);
        } else if (this->m_comp(value, this->m_pRoot->getDataRef())) {
            newNode->setNext(this->m_pRoot); newNode->setPrev(this->m_tail);
            this->m_pRoot->setPrev(newNode); this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        } else {
            Node *act = this->m_pRoot;
            while (act->getNext() != this->m_pRoot &&
                   !this->m_comp(value, act->getNext()->getDataRef()))
                act = act->getNext();
            Node *following = act->getNext();
            newNode->setNext(following); newNode->setPrev(act);
            act->setNext(newNode);       following->setPrev(newNode);
            if (act == this->m_tail) this->m_tail = newNode;
        }
        this->m_size++;
    }

    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, int direction, Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot || vueltas == 0) return;
        Node  *act   = (direction >= 0) ? this->m_pRoot : this->m_tail;
        size_t pasos = this->m_size * vueltas;
        for (size_t i = 0; i < pasos; ++i) {
            func(act->getDataRef(), forward<Args>(args)...);
            act = (direction >= 0) ? act->getNext() : act->getPrev();
        }
    }
};

#endif // __CIRCULARDOUBLELINKEDLIST_H__