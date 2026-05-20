#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"
#include <limits>

// Traits
template <typename T>
struct AscendingCLLTrait : BaseTrait<LLNode<T>, less<T>> {};

template <typename T>
struct DescendingCLLTrait : BaseTrait<LLNode<T>, greater<T>> {};

// Iterador circular: avanza con getNext() y detiene cuando regresa al nodo inicial
template <typename Container>
class CLLForwardIterator : public general_iterator<Container, CLLForwardIterator<Container>> {
public:
    using MySelf = CLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start;
    bool  m_started;
public:
    CLLForwardIterator(Container *pContainer, Node *pNode)
        : Parent(pContainer, pNode), m_start(pNode), m_started(false) {}
    CLLForwardIterator(Container *pContainer, Node *pNode, bool sentinel)
        : Parent(pContainer, pNode), m_start(nullptr), m_started(sentinel) {}

    MySelf operator++() {
        if(this->m_pNode) {
            m_started     = true;
            this->m_pNode = this->m_pNode->getNext();
            if(this->m_pNode == m_start)
                this->m_pNode = nullptr; // completó una vuelta → fin
        }
        return *this;
    }
};

template <typename Trait>
class CircularLinkedList : public LinkedList<Trait> {
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using MySelf           = CircularLinkedList<Trait>;
    using forward_iterator = CLLForwardIterator<MySelf>;
    friend forward_iterator;

    CircularLinkedList() {}

    // Copy constructor: do-while para recorrer la lista circular
    CircularLinkedList(const CircularLinkedList &other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        if(!other.m_pRoot) return;
        Node* curr = other.m_pRoot;
        do {
            push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        } while(curr != other.m_pRoot);
    }

    // Move constructor
    CircularLinkedList(CircularLinkedList &&other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = exchange(other.m_pRoot, nullptr);
        this->m_tail  = exchange(other.m_tail,  nullptr);
        this->m_size  = exchange(other.m_size,  0);
    }

    // Copy assignment
    CircularLinkedList& operator=(const CircularLinkedList &other) {
        if(this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            if(!other.m_pRoot) return *this;
            Node* curr = other.m_pRoot;
            do {
                push_back(curr->getData(), curr->getRef());
                curr = curr->getNext();
            } while(curr != other.m_pRoot);
        }
        return *this;
    }

    // Move assignment
    CircularLinkedList& operator=(CircularLinkedList &&other) {
        if(this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = exchange(other.m_pRoot, nullptr);
            this->m_tail  = exchange(other.m_tail,  nullptr);
            this->m_size  = exchange(other.m_size,  0);
        }
        return *this;
    }

    // Destructor: clear() limpia con lock; el padre recibe lista vacía
    virtual ~CircularLinkedList() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot) return;
        this->m_tail->setNext(nullptr); // rompe el círculo para recorrer linealmente
        Node* curr = this->m_pRoot;
        while(curr) {
            Node* next = curr->getNext();
            delete curr;
            curr = next;
        }
        this->m_pRoot = nullptr;
        this->m_tail  = nullptr;
        this->m_size  = 0;
    }

    // push_front: nuevo root, tail->next apunta a él
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref);
        if(!this->m_pRoot) {
            newNode->setNext(newNode);
            this->m_pRoot = this->m_tail = newNode;
        } else {
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    // push_back: nuevo tail apunta al root
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref);
        if(!this->m_pRoot) {
            newNode->setNext(newNode);
            this->m_pRoot = this->m_tail = newNode;
        } else {
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    // pop_front: repara el cierre circular con el nuevo root
    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot) throw runtime_error("La lista esta vacia");
        Node* temp = this->m_pRoot;
        auto result = make_tuple(temp->getData(), temp->getRef());
        if(this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            this->m_pRoot = temp->getNext();
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    // pop_back: repara el cierre circular con el nuevo tail
    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot) throw runtime_error("La lista esta vacia");
        Node* temp = this->m_tail;
        auto result = make_tuple(temp->getData(), temp->getRef());
        if(this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            Node* act = this->m_pRoot;
            while(act->getNext() != this->m_tail) act = act->getNext();
            this->m_tail = act;
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    // insert: inserta ordenado sin romper el enlace circular
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        cll_insert(value, ref);
    }

    forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr, true); }

    // ForEach: usa el iterador circular que para al completar la vuelta
    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(this->m_size == 0) return;
        for(auto& item : *this)
            func(item, forward<Args>(args)...);
    }

    // Mejora libre: recorre la lista N vueltas completas
    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot || vueltas == 0) return;
        Node*  act   = this->m_pRoot;
        size_t pasos = this->m_size * vueltas;
        for(size_t i = 0; i < pasos; ++i) {
            func(act->getDataRef(), forward<Args>(args)...);
            act = act->getNext();
        }
    }

    // operator<<: do-while circular, muestra ->root al final para confirmar el enlace
    friend ostream& operator<<(ostream& os, const CircularLinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "cll:[";
        if(list.m_pRoot) {
            Node* act = list.m_pRoot;
            do {
                os << "(" << act->getData() << "," << act->getRef() << ")";
                act = act->getNext();
                if(act != list.m_pRoot) os << "->";
            } while(act != list.m_pRoot);
            os << "]->root(" << list.m_pRoot->getData() << "," << list.m_pRoot->getRef() << ")";
        } else {
            os << "]";
        }
        return os;
    }

    // operator>>
    friend istream& operator>>(istream& is, CircularLinkedList& list) {
        char ch;
        if(!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type val;
        Ref ref;
        char comma, parenClose;
        while(is >> ch && ch != ']')
            if(ch == '(')
                if(is >> val >> comma >> ref >> parenClose)
                    if(comma == ',' && parenClose == ')')
                        list.insert(val, ref);
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }

private:
    // Inserción ordenada manteniendo el enlace circular intacto
    void cll_insert(const value_type &value, Ref ref) {
        Node* newNode = new Node(value, ref);
        this->m_size++;
        if(!this->m_pRoot) {
            newNode->setNext(newNode);
            this->m_pRoot = this->m_tail = newNode;
            return;
        }
        if(this->m_comp(value, this->m_pRoot->getDataRef())) {
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
            return;
        }
        Node* act = this->m_pRoot;
        while(act->getNext() != this->m_pRoot &&
              !this->m_comp(value, act->getNext()->getDataRef()))
            act = act->getNext();
        newNode->setNext(act->getNext());
        act->setNext(newNode);
        if(act == this->m_tail) this->m_tail = newNode;
    }
};

#endif // __CIRCULARLINKEDLIST_H__
