#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "doublelinkedlist.h"

template <typename T>
struct AscendingCDLLTrait : BaseTrait<T, less<T>>
{
  using Node = DLLNode<T>;
};

template <typename T>
struct DescendingCDLLTrait : BaseTrait<T, greater<T>>
{
  using Node = DLLNode<T>;
};

template <typename Container>
class CDLLForwardIterator
    : public general_iterator<Container, CDLLForwardIterator<Container>>
{
public:
  using MySelf = CDLLForwardIterator<Container>;
  using Parent = general_iterator<Container, MySelf>;
  using Node = typename Container::Node;

private:
  Node *m_pStart;

public:
  CDLLForwardIterator(Container *c, Node *node)
      : Parent(c, node), m_pStart(node) {}
  CDLLForwardIterator(Container *c, Node *node, bool)
      : Parent(c, node), m_pStart(nullptr) {}

  MySelf operator++()
  {
    if (this->m_pNode)
    {
      this->m_pNode = this->m_pNode->getNext();
      if (this->m_pNode == m_pStart)
        this->m_pNode = nullptr;
    }
    return *this;
  }
};

template <typename Container>
class CDLLBackwardIterator
    : public general_iterator<Container, CDLLBackwardIterator<Container>>
{
public:
  using MySelf = CDLLBackwardIterator<Container>;
  using Parent = general_iterator<Container, MySelf>;
  using Node = typename Container::Node;

private:
  Node *m_pStart;

public:
  CDLLBackwardIterator(Container *c, Node *node)
      : Parent(c, node), m_pStart(node) {}
  CDLLBackwardIterator(Container *c, Node *node, bool)
      : Parent(c, node), m_pStart(nullptr) {}

  MySelf operator++()
  {
    if (this->m_pNode)
    {
      this->m_pNode = this->m_pNode->getPrev();
      if (this->m_pNode == m_pStart)
        this->m_pNode = nullptr;
    }
    return *this;
  }
};

template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait>
{
public:
  using Base = DoubleLinkedList<Trait>;
  using value_type = typename Trait::value_type;
  using Node = typename Trait::Node; // DLLNode<T>

  using forward_iterator = CDLLForwardIterator<CircularDoubleLinkedList>;
  using backward_iterator = CDLLBackwardIterator<CircularDoubleLinkedList>;

  friend forward_iterator;
  friend backward_iterator;

  CircularDoubleLinkedList() : Base() {}

  void push_back(value_type value, Ref ref) override
  {
    unique_lock<shared_mutex> lock(this->m_mtx);
    Node *newNode = new Node(value, ref);
    if (this->m_size == 0)
    {
      this->m_pRoot = newNode;
      this->m_tail = newNode;
      newNode->setNext(newNode);
      newNode->setPrev(newNode);
    }
    else
    {
      newNode->setPrev(this->m_tail);
      newNode->setNext(this->m_pRoot);
      this->m_tail->setNext(newNode);
      this->m_pRoot->setPrev(newNode);
      this->m_tail = newNode;
    }
    this->m_size++;
  }

  void push_front(value_type value, Ref ref) override
  {
    unique_lock<shared_mutex> lock(this->m_mtx);
    Node *newNode = new Node(value, ref);
    if (this->m_size == 0)
    {
      this->m_pRoot = newNode;
      this->m_tail = newNode;
      newNode->setNext(newNode);
      newNode->setPrev(newNode);
    }
    else
    {
      newNode->setNext(this->m_pRoot);
      newNode->setPrev(this->m_tail);
      this->m_pRoot->setPrev(newNode);
      this->m_tail->setNext(newNode);
      this->m_pRoot = newNode;
    }
    this->m_size++;
  }

  forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
  forward_iterator end() { return forward_iterator(this, nullptr, true); }
  backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
  backward_iterator rend() { return backward_iterator(this, nullptr, true); }

  ~CircularDoubleLinkedList() override
  {
    if (this->m_tail)
      this->m_tail->setNext(nullptr);
    if (this->m_pRoot)
      this->m_pRoot->setPrev(nullptr);
  }
};

#endif // __CIRCULARDOUBLELINKEDLIST_H__
