#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

template <typename Container>
class DoubleLinkedListBackwardIterator : public general_iterator<Container, DoubleLinkedListBackwardIterator<Container>>{
public:
    using MySelf = DoubleLinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++() {
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getPrev();
        }
        return *this;
    }
};

template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>>{
public:
    using Node = DLLNode<T>;
    using Base = LLNode<T, Node>;
    using value_type = typename Base::value_type;

private:
    Node *m_pPrev;

public:
    DLLNode() : Base(), m_pPrev(nullptr) {}
    DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr)
        : Base(data, ref, next), m_pPrev(prev) {}

    Node*  getPrev() const     { return m_pPrev; }
    void   setPrev(Node *prev) { m_pPrev = prev; }
    Node*& getPrevRef()        { return m_pPrev; }
};

template <typename T>
struct AscendingDLLTrait : public BaseTrait<DLLNode<T>, less<T>>{
};

template <typename T>
struct DescendingDLLTrait : public BaseTrait<DLLNode<T>, greater<T>>{
};

template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait>{
public:
    using Base = LinkedList<Trait>;
    using value_type = typename Trait::value_type;
    using Node = typename Trait::Node;
    using MySelf = DoubleLinkedList<Trait>;
    using backward_iterator = DoubleLinkedListBackwardIterator<MySelf>;

    DoubleLinkedList() = default;

    DoubleLinkedList(const MySelf &other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node *curr = other.m_pRoot; curr != nullptr; curr = curr->getNext()) {
            push_back(curr->getData(), curr->getRef());
        }
    }

    DoubleLinkedList(MySelf &&other) {
        unique_lock<shared_mutex> lockOther(other.m_mtx);
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail = std::exchange(other.m_tail, nullptr);
        this->m_size = std::exchange(other.m_size, 0);
    }

    MySelf& operator=(const MySelf &other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            for (Node *curr = other.m_pRoot; curr != nullptr; curr = curr->getNext()) {
                push_back(curr->getData(), curr->getRef());
            }
        }
        return *this;
    }

    MySelf& operator=(MySelf &&other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lockOther(other.m_mtx);
            this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
            this->m_tail = std::exchange(other.m_tail, nullptr);
            this->m_size = std::exchange(other.m_size, 0);
        }
        return *this;
    }

    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    void push_front(value_type value, Ref ref) override;
    std::tuple<value_type, Ref> pop_front() override;
    void push_back(value_type value, Ref ref) override;
    std::tuple<value_type, Ref> pop_back() override;
    void insert(const value_type &value, Ref ref) override;

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        for (Node *curr = this->m_tail; curr != nullptr; curr = curr->getPrev()) {
            func(curr->getDataRef(), std::forward<Args>(args)...);
        }
    }

private:
    void clear();
};

template <typename Trait>
void DoubleLinkedList<Trait>::clear() {
    unique_lock<shared_mutex> lock(this->m_mtx);
    Node *curr = this->m_pRoot;
    while (curr) {
        Node *next = curr->getNext();
        delete curr;
        curr = next;
    }
    this->m_pRoot = nullptr;
    this->m_tail = nullptr;
    this->m_size = 0;
}

template <typename Trait>
void DoubleLinkedList<Trait>::push_front(value_type value, Ref ref) {
    unique_lock<shared_mutex> lock(this->m_mtx);
    Node *newNode = new Node(value, ref, this->m_pRoot, nullptr);

    if (this->m_pRoot) {
        this->m_pRoot->setPrev(newNode);
    } else {
        this->m_tail = newNode;
    }

    this->m_pRoot = newNode;
    this->m_size++;
}

template <typename Trait>
void DoubleLinkedList<Trait>::push_back(value_type value, Ref ref) {
    unique_lock<shared_mutex> lock(this->m_mtx);
    Node *newNode = new Node(value, ref, nullptr, this->m_tail);

    if (this->m_tail) {
        this->m_tail->setNext(newNode);
    } else {
        this->m_pRoot = newNode;
    }

    this->m_tail = newNode;
    this->m_size++;
}

template <typename Trait>
void DoubleLinkedList<Trait>::insert(const value_type &value, Ref ref) {
    unique_lock<shared_mutex> lock(this->m_mtx);
    Node *newNode = new Node(value, ref);

    if (!this->m_pRoot) {
        this->m_pRoot = newNode;
        this->m_tail = newNode;
        this->m_size++;
        return;
    }

    if (this->m_comp(value, this->m_pRoot->getDataRef())) {
        newNode->setNext(this->m_pRoot);
        this->m_pRoot->setPrev(newNode);
        this->m_pRoot = newNode;
        this->m_size++;
        return;
    }

    Node *curr = this->m_pRoot;
    while (curr->getNext() && !this->m_comp(value, curr->getNext()->getDataRef())) {
        curr = curr->getNext();
    }

    newNode->setNext(curr->getNext());
    newNode->setPrev(curr);

    if (curr->getNext()) {
        curr->getNext()->setPrev(newNode);
    } else {
        this->m_tail = newNode;
    }

    curr->setNext(newNode);
    this->m_size++;
}

template <typename Trait>
std::tuple<typename DoubleLinkedList<Trait>::value_type, Ref> DoubleLinkedList<Trait>::pop_front() {
    unique_lock<shared_mutex> lock(this->m_mtx);
    if (!this->m_pRoot) {
        throw runtime_error("La lista esta vacia");
    }

    Node *temp = this->m_pRoot;
    auto result = std::make_tuple(temp->getData(), temp->getRef());

    this->m_pRoot = temp->getNext();
    if (this->m_pRoot) {
        this->m_pRoot->setPrev(nullptr);
    } else {
        this->m_tail = nullptr;
    }

    delete temp;
    this->m_size--;
    return result;
}

template <typename Trait>
std::tuple<typename DoubleLinkedList<Trait>::value_type, Ref> DoubleLinkedList<Trait>::pop_back() {
    unique_lock<shared_mutex> lock(this->m_mtx);
    if (!this->m_tail) {
        throw runtime_error("La lista esta vacia");
    }

    Node *temp = this->m_tail;
    auto result = std::make_tuple(temp->getData(), temp->getRef());

    this->m_tail = temp->getPrev();
    if (this->m_tail) {
        this->m_tail->setNext(nullptr);
    } else {
        this->m_pRoot = nullptr;
    }

    delete temp;
    this->m_size--;
    return result;
}

#endif // __DOUBLELINKEDLIST_H__
