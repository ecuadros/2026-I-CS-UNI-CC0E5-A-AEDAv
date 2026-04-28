#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

template <typename Container>
class CircularForwardIterator : public general_iterator<Container, CircularForwardIterator<Container>> {
    bool m_isEnd = false;
public:
    using MySelf = CircularForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;

    CircularForwardIterator(Container* pContainer, typename Container::Node* pNode, bool isEnd = false)
        : Parent(pContainer, pNode), m_isEnd(isEnd) {}

    MySelf& operator++() {
        if (this->m_pNode == this->m_pContainer->getTail() || m_isEnd) {
            this->m_pNode = nullptr;
            m_isEnd = true;
        } else {
            this->m_pNode = this->m_pNode->getNext();
        }
        return *this;
    }

    bool operator==(const MySelf& other) const {
        if (m_isEnd && other.m_isEnd) return true;
        if (m_isEnd != other.m_isEnd) return false;
        return this->m_pNode == other.m_pNode;
    }
};

template <typename Trait>
class CircularLinkedList : public LinkedList<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using MySelf     = CircularLinkedList<Trait>;
    using Base       = LinkedList<Trait>;
    using iterator   = CircularForwardIterator<MySelf>;

    CircularLinkedList() : Base() {}
    CircularLinkedList(const CircularLinkedList& other) : Base() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        Node* pCurr = other.m_pRoot;
        for (size_t i = 0; i < other.m_size; ++i) {
            this->push_back(pCurr->getData(), pCurr->getRef());
            pCurr = pCurr->getNext();
        }
    }
    CircularLinkedList(CircularLinkedList&& other) noexcept : Base(std::move(other)) {}

    CircularLinkedList& operator=(const CircularLinkedList& other) {
        if (this != &other) {
            this->clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            Node* pCurr = other.m_pRoot;
            for (size_t i = 0; i < other.m_size; ++i) {
                this->push_back(pCurr->getData(), pCurr->getRef());
                pCurr = pCurr->getNext();
            }
        }
        return *this;
    }

    CircularLinkedList& operator=(CircularLinkedList&& other) noexcept {
        if (this != &other) {
            this->clear();
            Base::operator=(std::move(other));
        }
        return *this;
    }

    ~CircularLinkedList() override { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) return;

        Node* pCurr = this->m_pRoot;
        for (size_t i = 0; i < this->m_size; ++i) {
            Node* pNext = pCurr->getNext();
            delete pCurr;
            pCurr = pNext;
        }
        this->m_pRoot = this->m_tail = nullptr;
        this->m_size = 0;
    }

    iterator begin() { return iterator(this, this->m_pRoot, this->m_size == 0); }
    iterator end()   { return iterator(this, nullptr, true); }

    Node* getTail() const { return this->m_tail; }

    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, this->m_pRoot);
        if (!this->m_pRoot) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
        } else {
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, this->m_pRoot);
        if (!this->m_pRoot) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
        } else {
            this->m_tail->setNext(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

private:
    void internal_insert(Node* pCurr, Node* pPrev, const value_type& value, Ref ref) {
        if (pCurr == this->m_pRoot || this->m_comp(value, pCurr->getDataRef())) {
            Node* newNode = new Node(value, ref, pCurr);
            pPrev->setNext(newNode);
            if (pCurr == this->m_pRoot) this->m_tail = newNode;
            this->m_size++;
            return;
        }
        internal_insert(pCurr->getNext(), pCurr, value, ref);
    }

public:
    void insert(const value_type& value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot || this->m_comp(value, this->m_pRoot->getDataRef())) {
            lock.unlock(); // evitamos deadlock al llamar a push_front
            push_front(value, ref);
            return;
        }

        internal_insert(this->m_pRoot->getNext(), this->m_pRoot, value, ref);
    }

    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("Empty list");

        Node* pOldRoot = this->m_pRoot;
        auto res = make_tuple(pOldRoot->getData(), pOldRoot->getRef());

        if (this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            this->m_pRoot = pOldRoot->getNext();
            this->m_tail->setNext(this->m_pRoot);
        }

        delete pOldRoot;
        this->m_size--;
        return res;
    }

    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("Empty list");

        Node* pOldTail = this->m_tail;
        auto res = make_tuple(pOldTail->getData(), pOldTail->getRef());

        if (this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            Node* act = this->m_pRoot;
            while (act->getNext() != this->m_tail) {
                act = act->getNext();
            }
            this->m_tail = act;
            this->m_tail->setNext(this->m_pRoot);
        }

        delete pOldTail;
        this->m_size--;
        return res;
    }

    template <typename Func, typename... Args>
    void circularForEach(size_t loops, Func func, Args&&... args) {
        shared_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0 || loops == 0) return;
        Node* pCurr = this->m_pRoot;
        for (size_t i = 0; i < this->m_size * loops; ++i) {
            func(pCurr->getDataRef(), std::forward<Args>(args)...);
            pCurr = pCurr->getNext();
        }
    }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        for (auto it = begin(); it != end(); ++it) {
            func(*it, std::forward<Args>(args)...);
        }
    }

    friend ostream& operator<<(ostream& os, CircularLinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node* act = list.m_pRoot;
        for (size_t i = 0; i < list.m_size; ++i) {
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (i < list.m_size - 1) os << ",";
            act = act->getNext();
        }
        os << "]";
        return os;
    }
};

#endif
