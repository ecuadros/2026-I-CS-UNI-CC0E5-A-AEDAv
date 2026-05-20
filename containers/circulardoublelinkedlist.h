#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__
 
#include "doublelinkedlist.h"


// Traits de Ordenamiento
template <typename T>
struct AscendingCDLLTrait  : BaseTrait<DLLNode<T>, less<T>>{
};

template <typename T>
struct DescendingCDLLTrait : BaseTrait<DLLNode<T>, greater<T>>{
};

 
// CircularDoubleLinkedList
template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait>{
public:
    using value_type        = typename Trait::value_type;
    using Node              = typename Trait::Node;
    using Comp              = typename Trait::Comp;
    using MySelf            = CircularDoubleLinkedList<Trait>;
    using forward_iterator  = LinkedListForwardIterator <MySelf>;
    using backward_iterator = LinkedListBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;
 
    forward_iterator  begin()  { return forward_iterator (this, static_cast<Node*>(this->m_pRoot)); }
    forward_iterator  end()    { return forward_iterator (this, static_cast<Node*>(this->m_pRoot)); }
    backward_iterator rbegin() { return backward_iterator(this, static_cast<Node*>(this->m_tail));  }
    backward_iterator rend()   { return backward_iterator(this, static_cast<Node*>(this->m_tail));  }
 
    // Constructores
    CircularDoubleLinkedList() : DoubleLinkedList<Trait>() {}
 
    CircularDoubleLinkedList(const CircularDoubleLinkedList &other) : DoubleLinkedList<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        Node* curr = static_cast<Node*>(other.m_pRoot);
        for (size_t i = 0; i < other.m_size; i++, curr = curr->getNext())
            push_back(curr->getData(), curr->getRef());
    }
 
    CircularDoubleLinkedList(CircularDoubleLinkedList &&other) : DoubleLinkedList<Trait>() {
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail  = std::exchange(other.m_tail,  nullptr);
        this->m_size  = std::exchange(other.m_size,  0);
    }
 
    CircularDoubleLinkedList& operator=(const CircularDoubleLinkedList &other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            Node* curr = static_cast<Node*>(other.m_pRoot);
            for (size_t i = 0; i < other.m_size; i++, curr = curr->getNext())
                push_back(curr->getData(), curr->getRef());
        }
        return *this;
    }
 
    CircularDoubleLinkedList& operator=(CircularDoubleLinkedList &&other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
            this->m_tail  = std::exchange(other.m_tail,  nullptr);
            this->m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }
 
    // Destructor: rompe ambos ciclos antes de que DoubleLinkedList libere
    virtual ~CircularDoubleLinkedList() { clear(); }
 
    void clear() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        Node* tail = static_cast<Node*>(this->m_tail);
        tail->setNext(nullptr);                                  // rompe next circular
        static_cast<Node*>(this->m_pRoot)->setPrev(nullptr);    // rompe prev circular
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
            newNode->setPrev(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        } else {
            Node* tail = static_cast<Node*>(this->m_tail);
            Node* root = static_cast<Node*>(this->m_pRoot);
            tail->setNext(newNode);
            newNode->setPrev(tail);
            newNode->setNext(root);
            root->setPrev(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }
 
    // front
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref);
        if (this->m_size == 0) {
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        } else {
            Node* root = static_cast<Node*>(this->m_pRoot);
            Node* tail = static_cast<Node*>(this->m_tail);
            newNode->setNext(root);
            newNode->setPrev(tail);
            root->setPrev(newNode);
            tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }
 
    // inserta de manera ordenada 
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref);
        if (this->m_size == 0) {
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
            this->m_size++;
            return;
        }
        Node* act = static_cast<Node*>(this->m_pRoot);
        for (size_t i = 0; i < this->m_size; i++) {
            if (this->m_comp(value, act->getDataRef())) {
                Node* prev = act->getPrev();
                prev->setNext(newNode);
                newNode->setPrev(prev);
                newNode->setNext(act);
                act->setPrev(newNode);
                if (act == this->m_pRoot) this->m_pRoot = newNode;
                this->m_size++;
                return;
            }
            act = act->getNext();
        }
        // Inserta al final
        Node* tail = static_cast<Node*>(this->m_tail);
        Node* root = static_cast<Node*>(this->m_pRoot);
        tail->setNext(newNode);
        newNode->setPrev(tail);
        newNode->setNext(root);
        root->setPrev(newNode);
        this->m_tail = newNode;
        this->m_size++;
    }
 
    // ForEach: Se usa m_size como contador
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        Node* act = static_cast<Node*>(this->m_pRoot);
        for (size_t i = 0; i < this->m_size; i++, act = act->getNext())
            func(act->getDataRef(), std::forward<Args>(args)...);
    }
 
    // ReverseForEach: desde m_tail hacia atrás 
    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        Node* act = static_cast<Node*>(this->m_tail);
        for (size_t i = 0; i < this->m_size; i++, act = act->getPrev())
            func(act->getDataRef(), std::forward<Args>(args)...);
    }
    

    // size 
    size_t size() const override {
        shared_lock<shared_mutex> lock(this->m_mtx);
        return this->m_size;
    }

    protected:
    void do_print(ostream& os) const override {
        if (this->m_size == 0) return;
        Node* tail = static_cast<Node*>(this->m_tail);
        Node* root = static_cast<Node*>(this->m_pRoot);

        // romple los ciclos
        tail->setNext(nullptr); // rompe ciclo next
        root->setPrev(nullptr); // rompe ciclo prev

        // usa el do_print existente
        LinkedList<Trait>::do_print(os);

         // restaura
        tail->setNext(root);
        root->setPrev(tail);
    }

};
 
#endif