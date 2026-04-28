#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

// ----------- DDLNode ------------
template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>>{
    using Node = DLLNode<T>; 
private:
    Node *m_pPrev;
public:
    DLLNode() : LLNode<T, DLLNode<T>>(), m_pPrev(nullptr) {}
    DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr)
        : LLNode<T, DLLNode<T>>(data, ref, next), m_pPrev(prev) {}
    virtual ~DLLNode() {}

    Node*  getPrev() const     { return m_pPrev; }
    void   setPrev(Node *prev) { m_pPrev = prev; }
    Node*& getPrevRef()        { return m_pPrev; }
};

// ----------- Traits ------------
template <typename T>
struct AscendingDLLTrait : BaseTrait<T, less<T>>{
    using Node = DLLNode<T>;
};

template <typename T>
struct DescendingDLLTrait : BaseTrait<T, greater<T>>{
    using Node = DLLNode<T>;
};

// --------------------Forward - Backward Iterator ------------------
template <typename Container>
class DoubleLinkedListForwardIterator : public general_iterator<Container, DoubleLinkedListForwardIterator<Container>>{
public:
    using MySelf = DoubleLinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++(){
        if (this->m_pNode) this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
    bool operator!=(const MySelf& other) const {
        return !(*this == other);
    }
};

template <typename Container>
class DoubleLinkedListBackwardIterator : public general_iterator<Container, DoubleLinkedListBackwardIterator<Container>>{
public:
    using MySelf = DoubleLinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf operator++(){
        if (this->m_pNode) this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

// -------------------- DoubleLinkedList -------------------
template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait>{
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using Comp             = typename Trait::Comp;
    using MySelf           = DoubleLinkedList<Trait>;
    using forward_iterator  = DoubleLinkedListForwardIterator <MySelf>;
    using backward_iterator = DoubleLinkedListBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

    // ---- Iteradores ----
    forward_iterator  begin()  { return forward_iterator (this, static_cast<Node*>(this->m_pRoot)); }
    forward_iterator  end()    { return forward_iterator (this, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, static_cast<Node*>(this->m_tail)); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    // ---- Constructores ----
    DoubleLinkedList() : LinkedList<Trait>() {}

    DoubleLinkedList(const DoubleLinkedList &other) : LinkedList<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node* curr = static_cast<Node*>(other.m_pRoot); curr != nullptr; curr = curr->getNext())
            push_back(curr->getData(), curr->getRef());
    }

    DoubleLinkedList(DoubleLinkedList &&other) : LinkedList<Trait>(std::move(other)) {}

    DoubleLinkedList& operator=(const DoubleLinkedList &other) {
        if (this != &other) {
            while (this->m_size > 0) this->pop_front();
            shared_lock<shared_mutex> lock(other.m_mtx);
            for (Node* curr = static_cast<Node*>(other.m_pRoot); curr != nullptr; curr = curr->getNext())
                push_back(curr->getData(), curr->getRef());
        }
        return *this;
    }

    DoubleLinkedList& operator=(DoubleLinkedList &&other) {
        if (this != &other) {
            while (this->m_size > 0) this->pop_front();
            unique_lock<shared_mutex> lockOther(other.m_mtx);
            this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
            this->m_tail  = std::exchange(other.m_tail,  nullptr);
            this->m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    // ---- ForEach ----
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        for (auto& item : *this)
            func(item, std::forward<Args>(args)...);
    }

    // ---- push_back ----
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        } else {
            Node* tail = static_cast<Node*>(this->m_tail);
            tail->setNext(newNode);
            newNode->setPrev(tail);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    // ---- push_front ----
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        } else {
            Node* root = static_cast<Node*>(this->m_pRoot);
            newNode->setNext(root);
            root->setPrev(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    // ---- operator<< ----
    friend ostream& operator<<(ostream& os, const DoubleLinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node* act = static_cast<Node*>(list.m_pRoot);
        while (act) {
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (act->getNext()) os << ",";
            act = act->getNext();
        }
        os << "]";
        return os;
    }

    // ---- operator>> ----
    friend istream& operator>>(istream& is, DoubleLinkedList& list) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return is;
        }
        value_type val;
        Ref ref;
        char comma, parenClose;
        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref >> parenClose)
                    if (comma == ',' && parenClose == ')')
                        list.push_back(val, ref);
            }
        }
        return is;
    }
};

#endif