#ifndef BTREE_ITERATOR_H
#define BTREE_ITERATOR_H

#include <vector>
#include <utility>
#include <iterator>

template <typename TreeType>
class BTreeIteratorBase {
public:
    using value_type = typename TreeType::ObjectInfo;
    using reference  = value_type&;
    using pointer    = value_type*;
    using Page       = typename TreeType::BTPage;

protected:
    std::vector<std::pair<Page*, size_t>> m_stack;

public:
    BTreeIteratorBase() = default;

    reference operator*() { 
        return m_stack.back().first->m_Keys[m_stack.back().second]; 
    }
    
    pointer operator->() { 
        return &(operator*()); 
    }

    bool operator==(const BTreeIteratorBase& o) const {
        if (m_stack.empty() && o.m_stack.empty()) return true;
        if (m_stack.empty() || o.m_stack.empty()) return false;
        // Dos iteradores son iguales si están en la misma página y en el mismo índice
        return m_stack.back().first == o.m_stack.back().first &&
               m_stack.back().second == o.m_stack.back().second;
    }

    bool operator!=(const BTreeIteratorBase& o) const { 
        return !(*this == o); 
    }
};

template <typename TreeType, bool IsForward>
class BTreeIterator : public BTreeIteratorBase<TreeType> {
    using Page = typename TreeType::BTPage;

private:
    void Descend(Page* p) {
        while (p && p->m_KeyCount > 0) {
            // Evaluado en tiempo de compilación: 0 si es Forward, el último si es Backward
            size_t i = IsForward ? 0 : p->m_KeyCount - 1;
            this->m_stack.push_back({p, i});
            p = IsForward ? p->m_SubPages[0] : p->m_SubPages[p->m_KeyCount];
        }
    }

public:
    BTreeIterator(Page* root, bool atEnd) {
        if (!atEnd && root && root->m_KeyCount > 0) Descend(root);
    }

    BTreeIterator& operator++() {
        if (this->m_stack.empty()) return *this;

        Page* page = this->m_stack.back().first;
        size_t i   = this->m_stack.back().second;
        
        // Magia condicional estática
        Page* child = IsForward ? page->m_SubPages[i + 1] : page->m_SubPages[i];
        this->m_stack.back().second = IsForward ? i + 1 : i - 1; 
        
        if (child) {
            Descend(child);
        } else {
            while (!this->m_stack.empty() && 
                   this->m_stack.back().second >= this->m_stack.back().first->m_KeyCount) {
                this->m_stack.pop_back();
            }
        }
        return *this;
    }
};

template <typename TreeType>
using BTreeForwardIterator = BTreeIterator<TreeType, true>;

template <typename TreeType>
using BTreeBackwardIterator = BTreeIterator<TreeType, false>;

#endif