#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "doublelinkedlist.h"

template <typename Container>
class CircularForwardIterator {
public:
    using Node = typename Container::Node;
    using value_type = typename Container::value_type;
private:
    Node *m_pNode;
    size_t m_remaining;
public:
    CircularForwardIterator(Node *node, size_t remaining)
        : m_pNode(node), m_remaining(remaining) {}

    value_type& operator*() { return m_pNode->getDataRef(); }

    // LL Circular iteradpr
    CircularForwardIterator operator++() {
        if (m_remaining > 0) {
            m_pNode = m_pNode->getNext();
            --m_remaining;
        }
        return *this;
    }

    friend bool operator==(const CircularForwardIterator &a, const CircularForwardIterator &b) {
        return a.m_remaining == b.m_remaining;
    }
    friend bool operator!=(const CircularForwardIterator &a, const CircularForwardIterator &b) {
        return !(a == b);
    }
};

template <typename Trait>
class CircularLinkedList : public LinkedList<Trait> {
public:
    using Parent = LinkedList<Trait>;
    using value_type = typename Parent::value_type;
    using Node = typename Parent::Node;
    using forward_iterator = CircularForwardIterator<CircularLinkedList<Trait>>;

    CircularLinkedList() {}

    CircularLinkedList(const CircularLinkedList &other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        Node *curr = other.m_pRoot;
        for (size_t i = 0; i < other.m_size; ++i) {
            push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        }
    }

    ~CircularLinkedList() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_tail) {
            this->m_tail->setNext(nullptr);
        }
    }

    forward_iterator begin() { return forward_iterator(this->m_pRoot, this->m_size); }
    forward_iterator end() { return forward_iterator(nullptr, 0); }

    // insercion LL circular
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) {
            this->m_pRoot = new Node(value, ref);
            this->m_tail = this->m_pRoot;
            this->m_tail->setNext(this->m_pRoot);
            ++this->m_size;
            return;
        }

        Node *prev = nullptr;
        Node *curr = this->m_pRoot;
        for (size_t i = 0; i < this->m_size && !this->m_comp(value, curr->getDataRef()); ++i) {
            prev = curr;
            curr = curr->getNext();
        }

        Node *node = new Node(value, ref, curr);
        if (!prev) {
            node->setNext(this->m_pRoot);
            this->m_pRoot = node;
            this->m_tail->setNext(this->m_pRoot);
        } else {
            prev->setNext(node);
            if (prev == this->m_tail) {
                this->m_tail = node;
                this->m_tail->setNext(this->m_pRoot);
            }
        }
        ++this->m_size;
    }

    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *node = new Node(value, ref, this->m_pRoot);
        if (!this->m_pRoot) {
            this->m_pRoot = node;
            this->m_tail = node;
            node->setNext(node);
        } else {
            this->m_pRoot = node;
            this->m_tail->setNext(this->m_pRoot);
        }
        ++this->m_size;
    }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *node = new Node(value, ref, this->m_pRoot);
        if (!this->m_tail) {
            this->m_pRoot = node;
            this->m_tail = node;
            node->setNext(node);
        } else {
            this->m_tail->setNext(node);
            this->m_tail = node;
            this->m_tail->setNext(this->m_pRoot);
        }
        ++this->m_size;
    }

    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("La lista esta vacia");
        Node *temp = this->m_pRoot;
        auto result = make_tuple(temp->getData(), temp->getRef());
        if (this->m_pRoot == this->m_tail) {
            this->m_pRoot = nullptr;
            this->m_tail = nullptr;
        } else {
            this->m_pRoot = temp->getNext();
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        --this->m_size;
        return result;
    }

    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_tail) throw runtime_error("La lista esta vacia");
        Node *temp = this->m_tail;
        auto result = make_tuple(temp->getData(), temp->getRef());
        if (this->m_pRoot == this->m_tail) {
            this->m_pRoot = nullptr;
            this->m_tail = nullptr;
        } else {
            Node *prev = this->m_pRoot;
            while (prev->getNext() != this->m_tail) {
                prev = prev->getNext();
            }
            this->m_tail = prev;
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        --this->m_size;
        return result;
    }

    friend ostream& operator<<(ostream& os, const CircularLinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node *act = list.m_pRoot;
        for (size_t i = 0; i < list.m_size; ++i) {
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (i + 1 < list.m_size) os << ",";
            act = act->getNext();
        }
        os << "]";
        return os;
    }
};

template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using Parent = DoubleLinkedList<Trait>;
    using value_type = typename Parent::value_type;
    using Node = typename Parent::Node;
    using forward_iterator = CircularForwardIterator<CircularDoubleLinkedList<Trait>>;
    using backward_iterator = DoubleLinkedListBackwardIterator<CircularDoubleLinkedList<Trait>>;

    CircularDoubleLinkedList() {}

    ~CircularDoubleLinkedList() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_tail) this->m_tail->setNext(nullptr);
        if (this->m_pRoot) this->m_pRoot->setPrev(nullptr);
    }

    forward_iterator begin() { return forward_iterator(this->m_pRoot, this->m_size); }
    forward_iterator end() { return forward_iterator(nullptr, 0); }
    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend() { return backward_iterator(this, nullptr); }

    // Extra 2 validacion de enlaces CDLL
    bool validate_links() const {
        shared_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) {
            return this->m_pRoot == nullptr && this->m_tail == nullptr;
        }
        if (!this->m_pRoot || !this->m_tail) {
            return false;
        }
        if (this->m_tail->getNext() != this->m_pRoot) {
            return false;
        }
        if (this->m_pRoot->getPrev() != this->m_tail) {
            return false;
        }

        Node *curr = this->m_pRoot;
        for (size_t i = 0; i < this->m_size; ++i) {
            Node *next = curr->getNext();
            Node *prev = curr->getPrev();
            if (!next || !prev) {
                return false;
            }
            if (next->getPrev() != curr) {
                return false;
            }
            if (prev->getNext() != curr) {
                return false;
            }
            curr = next;
        }
        return curr == this->m_pRoot;
    }

    // Insercion CDLL
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) {
            Node *node = new Node(value, ref);
            this->m_pRoot = node;
            this->m_tail = node;
            node->setNext(node);
            node->setPrev(node);
            ++this->m_size;
            return;
        }

        Node *prev = nullptr;
        Node *curr = this->m_pRoot;
        for (size_t i = 0; i < this->m_size && !this->m_comp(value, curr->getDataRef()); ++i) {
            prev = curr;
            curr = curr->getNext();
        }
        if (!prev) prev = this->m_tail;

        Node *node = new Node(value, ref, curr, prev);
        prev->setNext(node);
        curr->setPrev(node);
        if (curr == this->m_pRoot && this->m_comp(value, curr->getDataRef())) {
            this->m_pRoot = node;
        }
        if (prev == this->m_tail && !this->m_comp(value, curr->getDataRef())) {
            this->m_tail = node;
        }
        this->m_pRoot->setPrev(this->m_tail);
        this->m_tail->setNext(this->m_pRoot);
        ++this->m_size;
    }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *node = new Node(value, ref);
        if (!this->m_tail) {
            this->m_pRoot = node;
            this->m_tail = node;
            node->setNext(node);
            node->setPrev(node);
        } else {
            node->setPrev(this->m_tail);
            node->setNext(this->m_pRoot);
            this->m_tail->setNext(node);
            this->m_pRoot->setPrev(node);
            this->m_tail = node;
        }
        ++this->m_size;
    }

    friend ostream& operator<<(ostream& os, const CircularDoubleLinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node *act = list.m_pRoot;
        for (size_t i = 0; i < list.m_size; ++i) {
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (i + 1 < list.m_size) os << ",";
            act = act->getNext();
        }
        os << "]";
        return os;
    }
};

#endif // __CIRCULARLINKEDLIST_H__
