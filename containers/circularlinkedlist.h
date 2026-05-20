#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

// Traits de Ordenamiento
template <typename T>
struct AscendingCLLTrait  : BaseTrait< LLNode<T>, less<T>>{
};

template <typename T>
struct DescendingCLLTrait : BaseTrait<LLNode<T>, greater<T>>{
};

// CircularLinkedList: hereda de LinkedList, pero push_back y push_front hacen el ciclo
template <typename Trait>
class CircularLinkedList : public LinkedList<Trait>{
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using Comp             = typename Trait::Comp;
    using MySelf           = CircularLinkedList<Trait>;
    using forward_iterator = LinkedListForwardIterator<MySelf>;
    friend forward_iterator;

    forward_iterator begin() { return forward_iterator(this, static_cast<Node*>(this->m_pRoot)); }
    forward_iterator end()   { return forward_iterator(this, static_cast<Node*>(this->m_pRoot)); }

    // Constructores
    CircularLinkedList() : LinkedList<Trait>() {}

    CircularLinkedList(const CircularLinkedList &other) : LinkedList<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        Node* curr = static_cast<Node*>(other.m_pRoot);
        for (size_t i = 0; i < other.m_size; i++, curr = curr->getNext())
            push_back(curr->getData(), curr->getRef());
    }

    CircularLinkedList(CircularLinkedList &&other) : LinkedList<Trait>() {
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail  = std::exchange(other.m_tail,  nullptr);
        this->m_size  = std::exchange(other.m_size,  0);
    }

    // operadores de asignacion
    CircularLinkedList& operator=(const CircularLinkedList &other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            Node* curr = static_cast<Node*>(other.m_pRoot);
            for (size_t i = 0; i < other.m_size; i++, curr = curr->getNext())
                push_back(curr->getData(), curr->getRef());
        }
        return *this;
    }

    CircularLinkedList& operator=(CircularLinkedList &&other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
            this->m_tail  = std::exchange(other.m_tail,  nullptr);
            this->m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    // Destructor: rompe el ciclo antes de que  ~LinkedList libere
    virtual ~CircularLinkedList() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        static_cast<Node*>(this->m_tail)->setNext(nullptr); // rompe el ciclo
        Node* act = static_cast<Node*>(this->m_pRoot);
        while (act) { Node* next = act->getNext(); delete act; act = next; }
        this->m_pRoot = nullptr;
        this->m_tail  = nullptr;
        this->m_size  = 0;
    }

    // push_back
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref);
        if (this->m_size == 0) {
            newNode->setNext(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        } else {
            static_cast<Node*>(this->m_tail)->setNext(newNode);
            newNode->setNext(static_cast<Node*>(this->m_pRoot));
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    // push_front
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, static_cast<Node*>(this->m_pRoot));
        if (this->m_size == 0) {
            newNode->setNext(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        } else {
            static_cast<Node*>(this->m_tail)->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    // insert ordenado
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref);
        if (this->m_size == 0) {
            newNode->setNext(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
            this->m_size++;
            return;
        }
        Node* act  = static_cast<Node*>(this->m_pRoot);
        Node* prev = static_cast<Node*>(this->m_tail);
        for (size_t i = 0; i < this->m_size; i++) {
            if (this->m_comp(value, act->getDataRef())) {
                prev->setNext(newNode);
                newNode->setNext(act);
                if (act == this->m_pRoot) this->m_pRoot = newNode;
                this->m_size++;
                return;
            }
            prev = act;
            act  = act->getNext();
        }
        static_cast<Node*>(this->m_tail)->setNext(newNode);
        newNode->setNext(static_cast<Node*>(this->m_pRoot));
        this->m_tail = newNode;
        this->m_size++;
    }
    
    // ForEach: probando -> m_size como contador, no nullptr 
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        Node* act = static_cast<Node*>(this->m_pRoot);
        for (size_t i = 0; i < this->m_size; i++, act = act->getNext())
            func(act->getDataRef(), std::forward<Args>(args)...);
    }  

    // size
    size_t size() const override {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return this->m_size;
    }

protected:
    // do_print: recorre con m_size porque tail->next != nullptr
    void do_print(ostream& os) const override {
        Node* act = static_cast<Node*>(this->m_pRoot);
        for (size_t i = 0; i < this->m_size; i++) {
            if (i > 0) os << ",";
            os << "(" << act->getData() << "," << act->getRef() << ")";
            act = act->getNext();
        }
    }
    // operator<< y operator>> se heredan de LinkedList
};

#endif