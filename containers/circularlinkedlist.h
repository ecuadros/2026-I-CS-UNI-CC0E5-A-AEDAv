#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

template <typename Container>
class CircularLinkedListIterator
    : public general_iterator<Container, CircularLinkedListIterator<Container>>
{
public:
  using MySelf = CircularLinkedListIterator<Container>;
  using Parent = general_iterator<Container, MySelf>;
  using Node = typename Container::Node;

private:
  Node *m_pStart;
  bool m_done;

public:
  CircularLinkedListIterator(Container *c, Node *node)
      : Parent(c, node), m_pStart(node), m_done(false) {}

  CircularLinkedListIterator(Container *c, Node *node, bool done)
      : Parent(c, node), m_pStart(nullptr), m_done(done) {}

  MySelf operator++()
  {
    if (this->m_pNode)
    {
      this->m_pNode = this->m_pNode->getNext();
      if (this->m_pNode == m_pStart)
      {
        this->m_pNode = nullptr;
      }
    }
    return *this;
  }
};

template <typename T>
struct AscendingCLLTrait : BaseTrait<T, less<T>>
{
  using Node = LLNode<T>;
};

template <typename T>
struct DescendingCLLTrait : BaseTrait<T, greater<T>>
{
  using Node = LLNode<T>;
};

template <typename Trait>
class CircularLinkedList : public LinkedList<Trait>
{
public:
  using Base = LinkedList<Trait>;
  using value_type = typename Trait::value_type;
  using Node = typename Trait::Node;
  using circular_iterator = CircularLinkedListIterator<CircularLinkedList>;

  friend circular_iterator;

  CircularLinkedList() : Base() {}

  void push_back(value_type value, Ref ref) override
  {
    unique_lock<shared_mutex> lock(this->m_mtx);
    Node *newNode = new Node(value, ref);
    if (this->m_size == 0)
    {
      this->m_pRoot = newNode;
      this->m_tail = newNode;
      newNode->setNext(newNode);
    }
    else
    {
      newNode->setNext(this->m_pRoot);
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
      newNode->setNext(newNode);
    }
    else
    {
      this->m_tail->setNext(newNode);
    }
    this->m_pRoot = newNode;
    this->m_size++;
  }

  std::tuple<value_type, Ref> pop_front() override
  {
    unique_lock<shared_mutex> lock(this->m_mtx);
    if (!this->m_pRoot)
      throw runtime_error("Lista vacía");

    Node *temp = this->m_pRoot;
    auto result = std::make_tuple(temp->getData(), temp->getRef());

    if (this->m_size == 1)
    {
      this->m_pRoot = nullptr;
      this->m_tail = nullptr;
    }
    else
    {
      this->m_pRoot = this->m_pRoot->getNext();
      this->m_tail->setNext(this->m_pRoot); // reestablece el anillo
    }
    delete temp;
    this->m_size--;
    return result;
  }

  circular_iterator begin() { return circular_iterator(this, this->m_pRoot); }
  circular_iterator end() { return circular_iterator(this, nullptr, true); }

  ~CircularLinkedList() override
  {
    if (this->m_tail)
      this->m_tail->setNext(nullptr);
  }
};

#endif // __CIRCULARLINKEDLIST_H__
