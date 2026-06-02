#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

// Reutilizacion de iteradores
template <typename Container>
class CLLForwardIterator : public general_iterator<Container, CLLForwardIterator<Container>> {
public:
    using MySelf = CLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;

private:
    Node* m_startNode;

public:
    CLLForwardIterator(Container *c, Node *node, Node *startNode) 
        : Parent(c, node), m_startNode(startNode) {}

    MySelf& operator++() {
        if (this->m_pNode) {
            Node *next    = this->m_pNode->getNext();
            this->m_pNode = (next == m_startNode) ? nullptr : next;
        }
        return *this;
    }
};

// Node en CLL
template <typename Trait>
class CircularLinkedList : public LinkedList<Trait> {
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using MySelf           = CircularLinkedList<Trait>;
    using forward_iterator = CLLForwardIterator<MySelf>;
    friend forward_iterator;

    CircularLinkedList() : LinkedList<Trait>() {}

    // destructor
    void clear() override {
        if (this->m_tail) this->m_tail->setNext(nullptr);
        LinkedList<Trait>::clear();
    }

    forward_iterator begin() const {
        return forward_iterator(const_cast<MySelf*>(this), this->m_pRoot, this->m_pRoot);
    }
    forward_iterator end() const {
        return forward_iterator(const_cast<MySelf*>(this), nullptr, this->m_pRoot);
    }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
        } else {
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
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
        } else {
            newNode->setNext(this->m_pRoot);
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
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        auto result = make_tuple(this->m_tail->getData(), this->m_tail->getRef());
        if (this->m_size == 1) {
            delete this->m_tail;
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            Node *act = this->m_pRoot;
            while (act->getNext() != this->m_tail) act = act->getNext();
            delete this->m_tail;
            this->m_tail = act;
            this->m_tail->setNext(this->m_pRoot);
        }
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
        } else if (this->m_comp(value, this->m_pRoot->getDataRef())) {
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        } else {
            Node *act = this->m_pRoot;
            while (act->getNext() != this->m_pRoot &&
                   !this->m_comp(value, act->getNext()->getDataRef()))
                act = act->getNext();
            newNode->setNext(act->getNext());
            act->setNext(newNode);
            if (act == this->m_tail) this->m_tail = newNode;
        }
        this->m_size++;
    }

    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0 || vueltas == 0) return;
        Node  *act   = this->m_pRoot;
        size_t pasos = this->m_size * vueltas;
        for (size_t i = 0; i < pasos; ++i) {
            func(act->getDataRef(), std::forward<Args>(args)...);
            act = act->getNext();
        }
    }
};

#endif // __CIRCULARLINKEDLIST_H__