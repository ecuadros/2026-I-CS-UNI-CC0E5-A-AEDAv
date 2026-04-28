#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "doublelinkedlist.h"
#include <limits>

// Traits
template <typename T>
struct AscendingCDLLTrait : BaseTrait<T, less<T>> {
    using Node = DLLNode<T>;
};

template <typename T>
struct DescendingCDLLTrait : BaseTrait<T, greater<T>> {
    using Node = DLLNode<T>;
};

// Iterador forward circular: para al volver al nodo inicial
template <typename Container>
class CDLLForwardIterator : public general_iterator<Container, CDLLForwardIterator<Container>> {
public:
    using MySelf = CDLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start;
public:
    CDLLForwardIterator(Container *pContainer, Node *pNode)
        : Parent(pContainer, pNode), m_start(pNode) {}
    CDLLForwardIterator(Container *pContainer, Node *pNode, bool sentinel)
        : Parent(pContainer, pNode), m_start(nullptr) {}

    MySelf operator++() {
        if(this->m_pNode) {
            this->m_pNode = this->m_pNode->getNext();
            if(this->m_pNode == m_start)
                this->m_pNode = nullptr;
        }
        return *this;
    }
};

// Iterador backward circular: retrocede con getPrev() y para al volver al inicio
template <typename Container>
class CDLLBackwardIterator : public general_iterator<Container, CDLLBackwardIterator<Container>> {
public:
    using MySelf = CDLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start;
public:
    CDLLBackwardIterator(Container *pContainer, Node *pNode)
        : Parent(pContainer, pNode), m_start(pNode) {}
    CDLLBackwardIterator(Container *pContainer, Node *pNode, bool sentinel)
        : Parent(pContainer, pNode), m_start(nullptr) {}

    MySelf operator++() {
        if(this->m_pNode) {
            this->m_pNode = this->m_pNode->getPrev();
            if(this->m_pNode == m_start)
                this->m_pNode = nullptr;
        }
        return *this;
    }
};

template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using value_type        = typename Trait::value_type;
    using Node              = typename Trait::Node;
    using MySelf            = CircularDoubleLinkedList<Trait>;
    using forward_iterator  = CDLLForwardIterator<MySelf>;
    using backward_iterator = CDLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

    CircularDoubleLinkedList() {}

    // Copy constructor: do-while para recorrer la lista circular
    CircularDoubleLinkedList(const CircularDoubleLinkedList &other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        if(!other.m_pRoot) return;
        Node* curr = other.m_pRoot;
        do {
            push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        } while(curr != other.m_pRoot);
    }

    // Move constructor
    CircularDoubleLinkedList(CircularDoubleLinkedList &&other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = exchange(other.m_pRoot, nullptr);
        this->m_tail  = exchange(other.m_tail,  nullptr);
        this->m_size  = exchange(other.m_size,  0);
    }

    // Copy assignment
    CircularDoubleLinkedList& operator=(const CircularDoubleLinkedList &other) {
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
    CircularDoubleLinkedList& operator=(CircularDoubleLinkedList &&other) {
        if(this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = exchange(other.m_pRoot, nullptr);
            this->m_tail  = exchange(other.m_tail,  nullptr);
            this->m_size  = exchange(other.m_size,  0);
        }
        return *this;
    }

    // Destructor: clear() rompe ambos enlaces y libera; el padre recibe lista vacía
    virtual ~CircularDoubleLinkedList() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot) return;
        this->m_tail->setNext(nullptr);
        this->m_pRoot->setPrev(nullptr);
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

    // push_front: mantiene m_pPrev del antiguo root + cierre circular
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, this->m_pRoot, this->m_tail);
        if(!this->m_pRoot) {
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_pRoot = this->m_tail = newNode;
        } else {
            this->m_pRoot->setPrev(newNode);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    // push_back: mantiene m_pPrev del nuevo tail + cierre circular
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref, this->m_pRoot, this->m_tail);
        if(!this->m_pRoot) {
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_pRoot = this->m_tail = newNode;
        } else {
            this->m_tail->setNext(newNode);
            this->m_pRoot->setPrev(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    // pop_front: repara next y prev circulares con el nuevo root
    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot) throw runtime_error("La lista esta vacia");
        Node* temp = this->m_pRoot;
        auto result = make_tuple(temp->getData(), temp->getRef());
        if(this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            this->m_pRoot = temp->getNext();
            this->m_pRoot->setPrev(this->m_tail);
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    // pop_back: retorna tuple, repara el cierre circular
    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot) throw runtime_error("La lista esta vacia");
        Node* temp = this->m_tail;
        auto result = make_tuple(temp->getData(), temp->getRef());
        if(this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            this->m_tail = temp->getPrev();
            this->m_tail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_tail);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    // insert: inserta ordenado manteniendo m_pPrev + cierre circular
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        cdll_insert(value, ref);
    }

    forward_iterator  begin()  { return forward_iterator (this, this->m_pRoot); }
    forward_iterator  end()    { return forward_iterator (this, nullptr, true); }
    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr, true); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(this->m_size == 0) return;
        for(auto& item : *this)
            func(item, forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(this->m_size == 0) return;
        for(auto it = rbegin(); it != rend(); ++it)
            func(*it, forward<Args>(args)...);
    }

    // Mejora libre: recorre N vueltas en la dirección indicada (>=0 forward, <0 backward)
    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, int direction, Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if(!this->m_pRoot || vueltas == 0) return;
        Node*  act   = (direction >= 0) ? this->m_pRoot : this->m_tail;
        size_t pasos = this->m_size * vueltas;
        for(size_t i = 0; i < pasos; ++i) {
            func(act->getDataRef(), forward<Args>(args)...);
            act = (direction >= 0) ? act->getNext() : act->getPrev();
        }
    }

    // operator<<: muestra fwd y bwd circular para verificar ambos enlaces
    friend ostream& operator<<(ostream& os, const CircularDoubleLinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "cdll fwd:[";
        if(list.m_pRoot) {
            Node* act = list.m_pRoot;
            do {
                os << "(" << act->getData() << "," << act->getRef() << ")";
                act = act->getNext();
                if(act != list.m_pRoot) os << "->";
            } while(act != list.m_pRoot);
            os << "]->root | bwd:[";
            act = list.m_tail;
            do {
                os << "(" << act->getData() << "," << act->getRef() << ")";
                act = act->getPrev();
                if(act != list.m_tail) os << "->";
            } while(act != list.m_tail);
            os << "]->tail";
        } else {
            os << "]";
        }
        return os;
    }

    // operator>>
    friend istream& operator>>(istream& is, CircularDoubleLinkedList& list) {
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
    // Inserción ordenada manteniendo m_pPrev y el enlace circular
    void cdll_insert(const value_type &value, Ref ref) {
        Node* newNode = new Node(value, ref);
        this->m_size++;
        if(!this->m_pRoot) {
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_pRoot = this->m_tail = newNode;
            return;
        }
        if(this->m_comp(value, this->m_pRoot->getDataRef())) {
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);
            this->m_pRoot->setPrev(newNode);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
            return;
        }
        Node* act = this->m_pRoot;
        while(act->getNext() != this->m_pRoot &&
              !this->m_comp(value, act->getNext()->getDataRef()))
            act = act->getNext();
        Node* nextNode = act->getNext();
        newNode->setNext(nextNode);
        newNode->setPrev(act);
        act->setNext(newNode);
        nextNode->setPrev(newNode);
        if(act == this->m_tail) this->m_tail = newNode;
    }
};

#endif // __CIRCULARDOUBLELINKEDLIST_H__
