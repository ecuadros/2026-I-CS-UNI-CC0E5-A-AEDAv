#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "doublelinkedlist.h"

//CDLLNode 
template <typename T>
class CDLLNode{
private:
    T        m_data;
    Ref      m_ref;
    CDLLNode *m_next;
    CDLLNode *m_prev;
public:
    CDLLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr), m_prev(nullptr) {}
    CDLLNode(T data, Ref ref, CDLLNode *next = nullptr, CDLLNode *prev = nullptr): m_data(data), m_ref(ref), m_next(next), m_prev(prev) {}
    virtual ~CDLLNode() {}
    T         getData()  const     { return m_data; }
    T        &getDataRef()         { return m_data; }
    void      setData(T data)      { m_data = data; }
    void      setRef(Ref ref)      { m_ref = ref; }
    Ref       getRef()   const     { return m_ref; }
    CDLLNode *getNext()  const     { return m_next; }
    CDLLNode *&getNextRef()        { return m_next; }
    void      setNext(CDLLNode *n) { m_next = n; }
    CDLLNode *getPrev()  const     { return m_prev; }
    CDLLNode *&getPrevRef()        { return m_prev; }
    void      setPrev(CDLLNode *p) { m_prev = p; }
};


//descending trait
template <typename T>
struct DescendingCDLLTrait : BaseTrait<T, greater<T>, CDLLNode<T>> {};


//Ascending trait
template <typename T>
struct AscendingCDLLTrait : BaseTrait<T, less<T>, CDLLNode<T>> {};


//forward iterator
template <typename Container>
class CDLLForwardIterator: public general_iterator<Container, CDLLForwardIterator<Container>>{
public:
    using MySelf = CDLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start;
public:
    CDLLForwardIterator(Container *pContainer, Node *pNode): Parent(pContainer, pNode), m_start(pNode) {}
    CDLLForwardIterator(Container *pContainer, Node *pNode, bool): Parent(pContainer, pNode), m_start(nullptr) {}
    //operador++ forward
    MySelf operator++(){
        if (this->m_pNode){
            this->m_pNode = this->m_pNode->getNext();
            if (this->m_pNode == m_start)
                this->m_pNode = nullptr;
        }
        return *this;
    }
};

//Backward iterator
template <typename Container>
class CDLLBackwardIterator: public general_iterator<Container, CDLLBackwardIterator<Container>>{
public:
    using MySelf = CDLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start;
public:
    CDLLBackwardIterator(Container *pContainer, Node *pNode): Parent(pContainer, pNode), m_start(pNode) {}
    CDLLBackwardIterator(Container *pContainer, Node *pNode, bool): Parent(pContainer, pNode), m_start(nullptr) {}
    //perator++ backward
    MySelf operator++(){
        if (this->m_pNode){
            this->m_pNode = this->m_pNode->getPrev();
            if (this->m_pNode == m_start)
                this->m_pNode = nullptr;
        }
        return *this;
    }
};


//CircularDoubleLinkedList
//hereda de LinkedList
template <typename Trait>
class CircularDoubleLinkedList : public LinkedList<Trait>{
public:
    using value_type = typename Trait::value_type;
    using Node = typename Trait::Node;
    using Comp = typename Trait::Comp;
    using MySelf = CircularDoubleLinkedList<Trait>;
    using forward_iterator = CDLLForwardIterator <MySelf>;
    using backward_iterator = CDLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;
private:
    Node  *m_pRoot = nullptr;
    Node  *m_tail  = nullptr;
    size_t m_size  = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;
    //Internal insert
    void internal_insert(const value_type &value, Ref ref){
        Node *newNode = new Node(value, ref);
        m_size++;
        //Lista vacia
        if (!m_pRoot){
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            m_pRoot = newNode;
            m_tail  = newNode;
            return;
        }
        //Nuevo head
        if (m_comp(value, m_pRoot->getDataRef())){
            newNode->setNext(m_pRoot);
            newNode->setPrev(m_tail);
            m_pRoot->setPrev(newNode);
            m_tail->setNext(newNode);
            m_pRoot = newNode;
            return;
        }
        //Buscar slot
        Node *act = m_pRoot;
        while (act->getNext() != m_pRoot && !m_comp(value, act->getNext()->getDataRef())){
            act = act->getNext();
        }
        Node *following = act->getNext();
        newNode->setNext(following);
        newNode->setPrev(act);
        act->setNext(newNode);
        following->setPrev(newNode);
        if (act == m_tail)
            m_tail = newNode;
    }

public:
    CircularDoubleLinkedList() : LinkedList<Trait>() {}
    //Copy constructor
    CircularDoubleLinkedList(const CircularDoubleLinkedList &other): LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0){
        shared_lock<shared_mutex> lock(other.m_mtx);
        if (!other.m_pRoot) return;
        Node *curr = other.m_pRoot;
        do {
            push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        } while (curr != other.m_pRoot);
    }

    //Move constructor
    CircularDoubleLinkedList(CircularDoubleLinkedList &&other): LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0){
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_tail  = std::exchange(other.m_tail,  nullptr);
        m_size  = std::exchange(other.m_size,  0);
    }

    //Copy assignment
    CircularDoubleLinkedList &operator=(const CircularDoubleLinkedList &other){
        if (this != &other){
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            if (!other.m_pRoot) return *this;
            Node *curr = other.m_pRoot;
            do {
                push_back(curr->getData(), curr->getRef());
                curr = curr->getNext();
            } while (curr != other.m_pRoot);
        }
        return *this;
    }

    //Move assignment
    CircularDoubleLinkedList &operator=(CircularDoubleLinkedList &&other){
        if (this != &other){
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_tail  = std::exchange(other.m_tail,  nullptr);
            m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }


    //Destructor seguro
    virtual ~CircularDoubleLinkedList() {
        clear();
    }

    void clear(){
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) return;
        m_tail->setNext(nullptr);
        Node *curr = m_pRoot;
        while (curr){
            Node *next = curr->getNext();
            delete curr;
            curr = next;
        }
        m_pRoot = nullptr;
        m_tail  = nullptr;
        m_size  = 0;
    }

    //push back
    void push_back(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (!m_pRoot){
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            m_pRoot = newNode;
            m_tail  = newNode;
        }
        else{
            newNode->setNext(m_pRoot);
            newNode->setPrev(m_tail);
            m_tail->setNext(newNode);
            m_pRoot->setPrev(newNode);
            m_tail = newNode;
        }
        m_size++;
    }


    //push front
    void push_front(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (!m_pRoot){
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            m_pRoot = newNode;
            m_tail  = newNode;
        }
        else{
            newNode->setNext(m_pRoot);
            newNode->setPrev(m_tail);
            m_pRoot->setPrev(newNode);
            m_tail->setNext(newNode);
            m_pRoot = newNode;
        }
        m_size++;
    }



    //Insert
    void insert(const value_type &value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(value, ref);
    }

    //Pop front: Ahora devuelve la tupla valor,ref
    std::tuple<value_type, Ref> pop_front() override{
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("lista vacia");
        Node *temp   = m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        if (m_size == 1){
            m_pRoot = nullptr;
            m_tail  = nullptr;
        }
        else{
            m_pRoot = temp->getNext();
            m_pRoot->setPrev(m_tail);
            m_tail->setNext(m_pRoot);
        }
        delete temp;
        m_size--;
        return result;
    }

    //Pop back: Ahora devuelve la tupla valor,ref
    std::tuple<value_type, Ref> pop_back() override{
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("listavacia");
        Node *temp   = m_tail;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        if (m_size == 1){
            m_pRoot = nullptr;
            m_tail  = nullptr;
        }
        else{
            m_tail = temp->getPrev();
            m_tail->setNext(m_pRoot);
            m_pRoot->setPrev(m_tail);
        }
        delete temp;
        m_size--;
        return result;
    }

    //Operator[]
    value_type &operator[](size_t index) override{
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("fuera de rango");
        Node *act = m_pRoot;
        for (size_t i = 0; i < index; ++i)
            act = act->getNext();
        return act->getDataRef();
    }

    //Size
    size_t size() const override{
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    //Iteradores
    forward_iterator  begin()  { return forward_iterator (this, m_pRoot); }
    forward_iterator  end()    { return forward_iterator (this, nullptr, true); }
    backward_iterator rbegin() { return backward_iterator(this, m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr, true); }

    //Foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        for (auto &item : *this)
            func(item, std::forward<Args>(args)...);
    }

    //ReverseForEach
    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it, std::forward<Args>(args)...);
    }

    //Circularforeach: N vueltas
    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, int direction, Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot || vueltas == 0) return;
        Node  *act   = (direction >= 0) ? m_pRoot : m_tail;
        size_t pasos = m_size * vueltas;
        for (size_t i = 0; i < pasos; ++i){
            func(act->getDataRef(), std::forward<Args>(args)...);
            act = (direction >= 0) ? act->getNext() : act->getPrev();
        }
    }

    string toString() const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        std::ostringstream os;

        os << "[";
        if (m_pRoot){
            Node *act = m_pRoot;
            do {
                os << "(" << act->getData() << "," << act->getRef() << ")";
                act = act->getNext();
                if (act != m_pRoot) os << ",";
            } while (act != m_pRoot);
        }
        os << "]";

        if (m_pRoot){
            os << " |bwd:[";
            Node *act = m_tail;
            do {
                os << "(" << act->getData() << "," << act->getRef() << ")";
                act = act->getPrev();
                if (act != m_tail) os << ",";
            } while (act != m_tail);

            os << "] ->root(" << m_pRoot->getData() << ")";
        }

        return os.str();
    }

    friend ostream &operator<<(ostream &os, const CircularDoubleLinkedList &list){
        return os << list.toString();
    }


    //Operator>> 
    friend istream &operator>>(istream &is, CircularDoubleLinkedList &list){
        char ch;
        if (!(is >> ch) || ch != '['){
            is.clear(ios_base::failbit);
            return is;
        }
        value_type val;
        Ref        ref;
        char       comma, parenClose;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> val >> comma >> ref >> parenClose)
                    if (comma == ',' && parenClose == ')')
                        list.insert(val, ref);
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }
};

#endif