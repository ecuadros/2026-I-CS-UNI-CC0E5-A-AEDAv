#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

// TODO Los iteradores ahora son forward y backward
// Crear 2 nuevos
template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>>
{
    using Node = DLLNode<T>;

private:
    Node *m_pPrev;

public:
    DLLNode() : LLNode<T, DLLNode<T>>(), m_pPrev(nullptr) {}
    DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr) : LLNode<T, DLLNode<T>>(data, ref, next), m_pPrev(prev) {}

    Node *getPrev() const { return m_pPrev; }
    void setPrev(Node *prev) { m_pPrev = prev; }
    Node *&getPrevRef() { return m_pPrev; }
};

template <typename T>
struct AscendingDLLTrait : BaseTrait<T, less<T>>
{
    using Node = DLLNode<T>;
};

template <typename T>
struct DescendingDLLTrait : BaseTrait<T, greater<T>>
{
    using Node = DLLNode<T>;
};

template <typename Container>
class DoubleLinkedListForwardIterator
    : public general_iterator<Container, DoubleLinkedListForwardIterator<Container>>
{
public:
    using MySelf = DoubleLinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++()
    {
        if (this->m_pNode)
            this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

template <typename Container>
class DoubleLinkedListBackwardIterator
    : public general_iterator<Container, DoubleLinkedListBackwardIterator<Container>>
{
public:
    using MySelf = DoubleLinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++()
    {
        if (this->m_pNode)
            this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait>
{

    // TODO: Copy constructor
    //       Simplificar y abstraer el bucle de copia de Nodes
    //       Es posible que no necesites este constructor ya que lo heredaste

    // TODO: Move constructor
    // TODO: Copy assignment operator
    // TODO: Move assignment operator
    using Base = LinkedList<Trait>;
    using forward_iterator = DoubleLinkedListForwardIterator<DoubleLinkedList>;
    using backward_iterator = DoubleLinkedListBackwardIterator<DoubleLinkedList>;

    friend forward_iterator;
    friend backward_iterator;

public:
    using value_type = typename Trait::value_type;
    using Node = typename Trait::Node;
    DoubleLinkedList() : Base() {}

    DoubleLinkedList(const DoubleLinkedList &other) : Base()
    {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node *curr = other.m_pRoot; curr != nullptr; curr = curr->getNext())
            push_back(curr->getData(), curr->getRef());
    }

    DoubleLinkedList(DoubleLinkedList &&other) : Base(std::move(other)) {}

    DoubleLinkedList &operator=(const DoubleLinkedList &other)
    {
        if (this != &other)
        {
            while (this->m_size > 0)
                this->pop_front();
            shared_lock<shared_mutex> lock(other.m_mtx);
            for (Node *curr = other.m_pRoot; curr != nullptr; curr = curr->getNext())
                push_back(curr->getData(), curr->getRef());
        }
        return *this;
    }

    DoubleLinkedList &operator=(DoubleLinkedList &&other)
    {
        Base::operator=(std::move(other));
        return *this;
    }

    void push_back(value_type value, Ref ref) override
    {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0)
        {
            this->m_pRoot = newNode;
            this->m_tail = newNode;
        }
        else
        {
            newNode->setPrev(this->m_tail);
            this->m_tail->setNext(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    void push_front(value_type value, Ref ref) override
    {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref, this->m_pRoot);
        if (this->m_size == 0)
        {
            this->m_tail = newNode;
        }
        else
        {
            this->m_pRoot->setPrev(newNode);
        }
        this->m_pRoot = newNode;
        this->m_size++;
    }

    forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
    forward_iterator end() { return forward_iterator(this, nullptr); }

    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend() { return backward_iterator(this, nullptr); }
};

#endif // __DOUBLELINKEDLIST_H__
