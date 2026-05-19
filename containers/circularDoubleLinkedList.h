#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "doublelinkedlist.h"

template <typename Container>
class CircularDLLForwardIterator : public general_iterator<Container, CircularDLLForwardIterator<Container>> {
    bool m_isEnd = false;
public:
    using MySelf = CircularDLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;

    CircularDLLForwardIterator(Container* pContainer, typename Container::Node* pNode, bool isEnd = false)
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

template <typename Container>
class CircularDLLBackwardIterator : public general_iterator<Container, CircularDLLBackwardIterator<Container>> {
    bool m_isEnd = false;
public:
    using MySelf = CircularDLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;

    CircularDLLBackwardIterator(Container* pContainer, typename Container::Node* pNode, bool isEnd = false)
        : Parent(pContainer, pNode), m_isEnd(isEnd) {}

    MySelf& operator++() {
        if (this->m_pNode == this->m_pContainer->getRoot() || m_isEnd) {
            this->m_pNode = nullptr;
            m_isEnd = true;
        } else {
            this->m_pNode = this->m_pNode->getPrev();
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
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using MySelf     = CircularDoubleLinkedList<Trait>;
    using Base       = DoubleLinkedList<Trait>;

    using forward_iterator  = CircularDLLForwardIterator<MySelf>;
    using backward_iterator = CircularDLLBackwardIterator<MySelf>;

    CircularDoubleLinkedList() : Base() {}
    CircularDoubleLinkedList(const CircularDoubleLinkedList& other) : Base() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        Node* pCurr = other.m_pRoot;
        for (size_t i = 0; i < other.m_size; ++i) {
            this->push_back(pCurr->getData(), pCurr->getRef());
            pCurr = pCurr->getNext();
        }
    }
    CircularDoubleLinkedList(CircularDoubleLinkedList&& other) noexcept : Base(std::move(other)) {}

    CircularDoubleLinkedList& operator=(const CircularDoubleLinkedList& other) {
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

    CircularDoubleLinkedList& operator=(CircularDoubleLinkedList&& other) noexcept {
        if (this != &other) {
            this->clear();
            Base::operator=(std::move(other));
        }
        return *this;
    }

    ~CircularDoubleLinkedList() override { clear(); }

    Node* getRoot() const { return this->m_pRoot; }
    Node* getTail() const { return this->m_tail; }

    forward_iterator  begin()  { return forward_iterator(this, this->m_pRoot, this->m_size == 0); }
    forward_iterator  end()    { return forward_iterator(this, nullptr, true); }
    backward_iterator rbegin() { return backward_iterator(this, this->m_tail, this->m_size == 0); }
    backward_iterator rend()   { return backward_iterator(this, nullptr, true); }

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

    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, this->m_pRoot, this->m_tail);
        if (!this->m_pRoot) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
        } else {
            this->m_pRoot->setPrev(newNode);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, this->m_pRoot, this->m_tail);
        if (!this->m_pRoot) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
        } else {
            this->m_tail->setNext(newNode);
            this->m_pRoot->setPrev(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
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
            this->m_pRoot->setPrev(this->m_tail);
            this->m_tail->setNext(this->m_pRoot);
        }

        delete pOldRoot;
        this->m_size--;
        return res;
    }

    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_tail) throw runtime_error("Empty list");

        Node* pOldTail = this->m_tail;
        auto res = make_tuple(pOldTail->getData(), pOldTail->getRef());

        if (this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            this->m_tail = pOldTail->getPrev();
            this->m_tail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_tail);
        }

        delete pOldTail;
        this->m_size--;
        return res;
    }

private:
    void internal_insert(Node* pCurr, const value_type& value, Ref ref) {
        if (pCurr == this->m_pRoot || this->m_comp(value, pCurr->getDataRef())) {
            Node* newNode = new Node(value, ref, pCurr, pCurr->getPrev());
            pCurr->getPrev()->setNext(newNode);
            pCurr->setPrev(newNode);
            if (pCurr == this->m_pRoot) this->m_tail = newNode;
            this->m_size++;
            return;
        }
        internal_insert(pCurr->getNext(), value, ref);
    }

public:
    void insert(const value_type& value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot || this->m_comp(value, this->m_pRoot->getDataRef())) {
            lock.unlock();
            push_front(value, ref);
            return;
        }

        internal_insert(this->m_pRoot->getNext(), value, ref);
    }
    template <typename Func, typename... Args>
    void circularForEach(size_t loops, int direction, Func func, Args&&... args) {
        shared_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0 || loops == 0) return;
        Node* pCurr = (direction >= 0) ? this->m_pRoot : this->m_tail;
        for (size_t i = 0; i < this->m_size * loops; ++i) {
            func(pCurr->getDataRef(), std::forward<Args>(args)...);
            pCurr = (direction >= 0) ? pCurr->getNext() : pCurr->getPrev();
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

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args&&... args) {
        unique_lock<std::shared_mutex> lock(this->m_mtx);
        for (auto it = rbegin(); it != rend(); ++it) {
            func(*it, std::forward<Args>(args)...);
        }
    }

    friend ostream& operator<<(ostream& os, CircularDoubleLinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node* pCurr = list.m_pRoot;
        for (size_t i = 0; i < list.m_size; ++i) {
            os << "(" << pCurr->getData() << "," << pCurr->getRef() << ")";
            if (i < list.m_size - 1) os << ",";
            pCurr = pCurr->getNext();
        }
        os << "]";
        return os;
    }
};

#endif
