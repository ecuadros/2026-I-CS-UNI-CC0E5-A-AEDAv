#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"
#include <limits>

using namespace std;

template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>> {
    using Node = DLLNode<T>;
    using Base = LLNode<T, Node>;
private:
    Node* m_pPrev = nullptr;
public:
    DLLNode() : Base(), m_pPrev(nullptr) {}
    DLLNode(T data, Ref ref, Node* next = nullptr, Node* prev = nullptr) : Base(data, ref, next), m_pPrev(prev) {}
    ~DLLNode() override {}
    Node*  getPrev() const     { return m_pPrev; }
    void   setPrev(Node* prev) { m_pPrev = prev; }
    Node*& getPrevRef()        { return m_pPrev; }
};

template <typename T>
struct AscendingDLLTrait : BaseTrait<T, less<T>, DLLNode<T>> {};

template <typename T>
struct DescendingDLLTrait : BaseTrait<T, greater<T>, DLLNode<T>> {};

template <typename Container>
class DLLForwardIterator : public general_iterator<Container, DLLForwardIterator<Container>> {
public:
    using MySelf = DLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() {
        if (this->m_pNode) this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

template <typename Container>
class DLLBackwardIterator : public general_iterator<Container, DLLBackwardIterator<Container>> {
public:
    using MySelf = DLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() {
        if (this->m_pNode) this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait> {
public:
    using value_type        = typename Trait::value_type;
    using Node              = typename Trait::Node;
    using Comp              = typename Trait::Comp;
    using MySelf            = DoubleLinkedList<Trait>;
    using Base              = LinkedList<Trait>;

    using forward_iterator  = DLLForwardIterator<MySelf>;
    using backward_iterator = DLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

private:
    void internal_insert(Node*& pCurr, Node* pPrev, const value_type& value, Ref ref) {
        if (!pCurr || this->m_comp(value, pCurr->getDataRef())) {
            Node* newNode = new Node(value, ref, pCurr, pPrev);
            if (pCurr) pCurr->setPrev(newNode);
            pCurr = newNode;
            this->m_size++;
            if (newNode->getNext() == nullptr) this->m_tail = newNode;
            return;
        }
        internal_insert(pCurr->getNextRef(), pCurr, value, ref);
    }

    void clear() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* pCurr = this->m_pRoot;
        while (pCurr) {
            Node* next = pCurr->getNext();
            delete pCurr;
            pCurr = next;
        }
        this->m_pRoot = nullptr;
        this->m_tail  = nullptr;
        this->m_size  = 0;
    }

public:
    DoubleLinkedList() : Base() {}
    DoubleLinkedList(const DoubleLinkedList& other) : Base() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node* pCurr = other.m_pRoot; pCurr != nullptr; pCurr = pCurr->getNext())
            this->push_back(pCurr->getData(), pCurr->getRef());
    }
    DoubleLinkedList(DoubleLinkedList&& other) noexcept : Base(move(other)) {}
    ~DoubleLinkedList() override {
        clear();
    }

    forward_iterator  begin()  { return forward_iterator(this, this->m_pRoot); }
    forward_iterator  end()    { return forward_iterator(this, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, this->m_pRoot, nullptr);
        if (this->m_pRoot)
            this->m_pRoot->setPrev(newNode);
        else
            this->m_tail = newNode;
        this->m_pRoot = newNode;
        this->m_size++;
    }

    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("ERROR: Empty list");
        Node* tmp = this->m_pRoot;
        auto  out = make_tuple(tmp->getData(), tmp->getRef());
        this->m_pRoot = tmp->getNext();
        if (this->m_pRoot)
            this->m_pRoot->setPrev(nullptr);
        else
            this->m_tail = nullptr;
        delete tmp;
        this->m_size--;
        return out;
    }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, nullptr, this->m_tail);
        if (this->m_tail)
            this->m_tail->setNext(newNode);
        else
            this->m_pRoot = newNode;
        this->m_tail = newNode;
        this->m_size++;
    }

    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_tail) throw std::runtime_error("ERROR: Empty list");
        Node* tmp = this->m_tail;
        auto  out = make_tuple(tmp->getData(), tmp->getRef());
        this->m_tail = tmp->getPrev();
        if (this->m_tail)
            this->m_tail->setNext(nullptr);
        else
            this->m_pRoot = nullptr;
        delete tmp;
        this->m_size--;
        return out;
    }

    void insert(const value_type& value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        internal_insert(this->m_pRoot, nullptr, value, ref);
        if (this->m_size == 1) this->m_tail = this->m_pRoot;
    }

    MySelf& operator=(const MySelf& other) {
        if (this != &other) {
            this->clear();
            shared_lock<shared_mutex> lockOther(other.m_mtx);
            for (Node* pCurr = other.m_pRoot; pCurr != nullptr; pCurr = pCurr->getNext())
                this->push_back(pCurr->getData(), pCurr->getRef());
        }
        return *this;
    }

    MySelf& operator=(MySelf&& other) noexcept {
        if (this != &other) {
            this->clear();
            unique_lock<shared_mutex> lockOther(other.m_mtx);
            this->m_pRoot = exchange(other.m_pRoot, nullptr);
            this->m_tail  = exchange(other.m_tail, nullptr);
            this->m_size  = exchange(other.m_size, 0);
        }
        return *this;
    }

    value_type& operator[](size_t index) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (index >= this->m_size) throw out_of_range("Index out of range");
        Node* pCurr = this->m_pRoot;
        if (index < this->m_size / 2) {
            for (size_t i = 0; i < index; ++i)
                pCurr = pCurr->getNext();
        } else {
            pCurr = this->m_tail;
            for (size_t i = this->m_size - 1; i > index; --i)
                pCurr = pCurr->getPrev();
        }
        return pCurr->getDataRef();
    }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args&&... args) {
        unique_lock<std::shared_mutex> lock(this->m_mtx);
        for (auto it = rbegin(); it != rend(); ++it) {
            func(*it, std::forward<Args>(args)...);
        }
    }

    friend ostream& operator<<(ostream& os, const DoubleLinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node* pCurr = list.m_pRoot;
        while (pCurr) {
            os << "(" << pCurr->getData() << "," << pCurr->getRef() << ")";
            if (pCurr->getNext()) os << ",";
            pCurr = pCurr->getNext();
        }
        os << "]";
        return os;
    }

    friend istream& operator>>(istream& is, DoubleLinkedList& list) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return is;
        }
        value_type val;
        Ref ref;
        char comma, parentClose;
        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref >> parentClose) {
                    if (comma == ',' && parentClose == ')') {
                        list.insert(val, ref);
                    }
                }
            }
        }
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }
};

#endif // __DOUBLELINKEDLIST_H__
