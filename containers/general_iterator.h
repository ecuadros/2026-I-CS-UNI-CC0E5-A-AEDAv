#ifndef __ITERATOR_H__
#define __ITERATOR_H__
#include <algorithm>
#include <utility>

template <typename Container, class IteratorBase> // 
class general_iterator
{public:
    using Node = typename Container::Node;
    using value_type = typename Container::value_type;
    using myself = general_iterator<Container, IteratorBase>;

protected:
    Container *m_pContainer;
    Node      *m_pNode;
public:
    general_iterator(Container *pContainer, Node *pNode)
        : m_pContainer(pContainer), m_pNode(pNode) {}
    general_iterator(const myself &other) 
          : m_pContainer(other.m_pContainer), m_pNode(other.m_pNode){}
    general_iterator(myself &&other) // Move constructor
          {   m_pContainer = move(other.m_pContainer);
              m_pNode      = move(other.m_pNode);
          }
    IteratorBase &operator=(IteratorBase &iter)
          {   m_pContainer = move(iter.m_pContainer);
              m_pNode      = move(iter.m_pNode);
              return *(IteratorBase *)this; // Pending static_cast?
          }
    Node *getNode() const { return m_pNode; }
    friend bool operator==(const IteratorBase &a, const IteratorBase &b) { return a.getNode() == b.getNode(); }
    friend bool operator!=(const IteratorBase &a, const IteratorBase &b) { return a.getNode() != b.getNode(); }

    value_type &operator*() { return m_pNode->getDataRef(); }

    IteratorBase &operator++() {
        if (m_pNode) m_pNode = m_pNode->getNext();
        return *(IteratorBase *)this;
    }
};

template <typename Container, class IteratorBase>
class circular_iterator : public general_iterator<Container, IteratorBase> {
public:
    using Node = typename Container::Node;
protected:
    Node *m_pRoot;
public://Se agrego circular_iterator
    circular_iterator(Container *pContainer, Node *pNode, Node *pRoot): general_iterator<Container, IteratorBase>(pContainer, pNode),m_pRoot(pRoot) {}
    IteratorBase &operator++() {
        if (this->m_pNode) {
            Node *next    = this->m_pNode->getNext();
            this->m_pNode = (next == m_pRoot) ? nullptr : next;
        }
        return *(IteratorBase *)this;
    }
};

#endif
 