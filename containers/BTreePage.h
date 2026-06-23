#ifndef BTREE_PAGE_H
#define BTREE_PAGE_H

#include <functional>
#include <iostream>
#include <utility>
#include <vector>

#include "../types.h"
#include "traits.h"

using namespace std;

template <typename Trait>
class BTree;

template <typename Trait>
struct BTreeEntry {
    using value_type = typename Trait::value_type;

    value_type m_data{};
    Ref m_ref{};
    Size m_useCount{0};

    BTreeEntry() = default;

    BTreeEntry(const value_type& data, Ref ref)
        : m_data(data), m_ref(ref) {}

    operator const value_type&() const { return m_data; }

    Size touch() { return ++m_useCount; }
    Size useCount() const { return m_useCount; }

    friend ostream& operator<<(ostream& os, const BTreeEntry& entry) {
        return os << entry.m_data << ':' << entry.m_ref;
    }

    friend istream& operator>>(istream& is, BTreeEntry& entry) {
        Token separator{};
        if (!(is >> entry.m_data >> separator >> entry.m_ref) || separator != ':')
            is.setstate(ios_base::failbit);
        return is;
    }
};

template <typename Trait>
class BTreePage {
public:
    using value_type = typename Trait::value_type;
    using Comp = typename Trait::Comp;
    using Entry = BTreeEntry<Trait>;
    using Page = BTreePage<Trait>;

    static constexpr Size Order = Trait::Order;

private:
    friend class BTree<Trait>;

    Comp m_comp{};
    vector<Entry> m_keys;
    vector<Page*> m_subPages;
    Size m_keyCount{0};
    Size m_maxKeys;
    Size m_maxKeysForChildren;
    Flag m_unique;

    Flag equivalent(const value_type& a, const value_type& b) const {
        return !invoke(m_comp, a, b) && !invoke(m_comp, b, a);
    }

    Size locate(const value_type& key) const {
        Size first = 0;
        Size last = m_keyCount;

        while (first < last) {
            Size mid = first + (last - first) / 2;
            if (invoke(m_comp, m_keys[mid].m_data, key))
                first = mid + 1;
            else
                last = mid;
        }
        return first;
    }

    template <typename Container, typename Item>
    static void insertAt(Container& c, const Item& item, Size pos) {
        SIndex last = static_cast<SIndex>(c.size()) - 2;
        SIndex target = static_cast<SIndex>(pos);

        for (SIndex i = last; i >= target; --i)
            c[static_cast<Size>(i + 1)] = c[static_cast<Size>(i)];

        c[pos] = item;
    }

    template <typename Container>
    static void removeAt(Container& c, Size pos) {
        for (Size i = pos + 1; i < c.size(); ++i)
            c[i - 1] = c[i];
    }

    Size freeCells() const { return m_maxKeys - m_keyCount; }
    Flag isFull() const { return m_keyCount >= m_maxKeys; }
    Flag isOverflow() const { return m_keyCount > m_maxKeys; }
    Size minKeys() const { return 2 * m_maxKeys / 3; }

    Size freeCellsOnLeft(Size position) const {
        return position > 0
            ? m_subPages[position - 1]->freeCells()
            : 0;
    }

    Size freeCellsOnRight(Size position) const {
        return position < m_keyCount
            ? m_subPages[position + 1]->freeCells()
            : 0;
    }

    void setMaxKeysForChildren(Size order) {
        m_maxKeysForChildren = order;
    }

    void create() {
        m_keys.assign(m_maxKeys + 1, Entry{});
        m_subPages.assign(m_maxKeys + 2, nullptr);
        m_keyCount = 0;
    }

    void clearKeys() {
        m_keyCount = 0;
    }

    void reset() {
        for (Size index = 0; index <= m_keyCount; ++index) {
            delete m_subPages[index];
            m_subPages[index] = nullptr;
        }
        clearKeys();
    }

    Flag redistribute(Size pos) {
        Size left = freeCellsOnLeft(pos);
        Size right = freeCellsOnRight(pos);

        if (left == 0 && right == 0)
            return false;

        if (left > right)
            redistributeRightToLeft(pos);
        else
            redistributeLeftToRight(pos);
        return true;
    }

    void redistributeRightToLeft(Size position) {
        Page* source = m_subPages[position];
        Page* target = m_subPages[position - 1];

        while (source->m_keyCount > source->minKeys()
               && target->m_keyCount < source->m_keyCount) {
            insertAt(
                target->m_keys,
                m_keys[position - 1],
                target->m_keyCount++);
            insertAt(
                target->m_subPages,
                source->m_subPages[0],
                target->m_keyCount);

            m_keys[position - 1] = source->m_keys[0];
            removeAt(source->m_keys, 0);
            removeAt(source->m_subPages, 0);
            --source->m_keyCount;
        }
    }

    void redistributeLeftToRight(Size position) {
        Page* source = m_subPages[position];
        Page* target = m_subPages[position + 1];

        while (source->m_keyCount > source->minKeys()
               && target->m_keyCount < source->m_keyCount) {
            insertAt(target->m_keys, m_keys[position], 0);
            insertAt(
                target->m_subPages,
                source->m_subPages[source->m_keyCount],
                0);
            ++target->m_keyCount;

            m_keys[position] = source->m_keys[source->m_keyCount - 1];
            --source->m_keyCount;
        }
    }

    void movePage(
        Page* child,
        vector<Entry>& temporaryKeys,
        vector<Page*>& temporaryChildren) {
        const Size count = child->m_keyCount;

        for (Size index = 0; index < count; ++index) {
            temporaryKeys.push_back(child->m_keys[index]);
            temporaryChildren.push_back(child->m_subPages[index]);
            child->m_subPages[index] = nullptr;
        }

        temporaryChildren.push_back(child->m_subPages[count]);
        child->m_subPages[count] = nullptr;
        child->clearKeys();
    }

    void splitIntoThree(
        vector<Entry>& temporaryKeys,
        vector<Page*>& temporaryChildren,
        Page*& firstChild,
        Page*& secondChild,
        Page*& thirdChild,
        Entry& firstPromoted,
        Entry& secondPromoted) {
        if (!firstChild)
            firstChild = new Page(m_maxKeysForChildren, m_unique);

        firstChild->clearKeys();
        Size childKeyCount = (temporaryKeys.size() - 2) / 3;
        Size sourceIndex = 0;

        for (; sourceIndex < childKeyCount; ++sourceIndex) {
            firstChild->m_keys[sourceIndex] =
                temporaryKeys[sourceIndex];
            firstChild->m_subPages[sourceIndex] =
                temporaryChildren[sourceIndex];
            ++firstChild->m_keyCount;
        }
        firstChild->m_subPages[sourceIndex] =
            temporaryChildren[sourceIndex];
        firstPromoted = temporaryKeys[sourceIndex++];

        if (!secondChild)
            secondChild = new Page(m_maxKeysForChildren, m_unique);

        secondChild->clearKeys();
        childKeyCount += (temporaryKeys.size() - 2) / 3 + 1;
        Size targetIndex = 0;

        for (; sourceIndex < childKeyCount;
             ++sourceIndex, ++targetIndex) {
            secondChild->m_keys[targetIndex] =
                temporaryKeys[sourceIndex];
            secondChild->m_subPages[targetIndex] =
                temporaryChildren[sourceIndex];
            ++secondChild->m_keyCount;
        }
        secondChild->m_subPages[targetIndex] =
            temporaryChildren[sourceIndex];
        secondPromoted = temporaryKeys[sourceIndex++];

        if (!thirdChild)
            thirdChild = new Page(m_maxKeysForChildren, m_unique);

        thirdChild->clearKeys();
        for (targetIndex = 0;
             sourceIndex < temporaryKeys.size();
             ++sourceIndex, ++targetIndex) {
            thirdChild->m_keys[targetIndex] =
                temporaryKeys[sourceIndex];
            thirdChild->m_subPages[targetIndex] =
                temporaryChildren[sourceIndex];
            ++thirdChild->m_keyCount;
        }
        thirdChild->m_subPages[targetIndex] =
            temporaryChildren[sourceIndex];
    }

    void splitChild(Size position) {
        Page* firstChild = nullptr;
        Page* secondChild = nullptr;

        if (position > 0 && m_subPages[position - 1]->isFull()) {
            firstChild = m_subPages[position - 1];
            secondChild = m_subPages[position--];
        }

        if (position < m_keyCount
            && m_subPages[position + 1]->isFull()) {
            firstChild = m_subPages[position];
            secondChild = m_subPages[position + 1];
        }

        vector<Entry> temporaryKeys;
        vector<Page*> temporaryChildren;
        movePage(firstChild, temporaryKeys, temporaryChildren);
        temporaryKeys.push_back(m_keys[position]);
        movePage(secondChild, temporaryKeys, temporaryChildren);

        Page* thirdChild = nullptr;
        Entry firstPromoted;
        Entry secondPromoted;
        splitIntoThree(
            temporaryKeys,
            temporaryChildren,
            firstChild,
            secondChild,
            thirdChild,
            firstPromoted,
            secondPromoted);

        m_keys[position] = firstPromoted;
        m_subPages[position] = firstChild;
        insertAt(m_keys, secondPromoted, position + 1);
        insertAt(m_subPages, secondChild, position + 1);
        ++m_keyCount;
        m_subPages[position + 2] = thirdChild;
    }

    void splitRoot() {
        Page* firstChild = nullptr;
        Page* secondChild = nullptr;
        Page* thirdChild = nullptr;
        Entry firstPromoted;
        Entry secondPromoted;

        splitIntoThree(
            m_keys,
            m_subPages,
            firstChild,
            secondChild,
            thirdChild,
            firstPromoted,
            secondPromoted);

        clearKeys();
        m_keys[0] = firstPromoted;
        m_subPages[0] = firstChild;
        ++m_keyCount;
        m_keys[1] = secondPromoted;
        m_subPages[1] = secondChild;
        ++m_keyCount;
        m_subPages[2] = thirdChild;
    }

public:
    explicit BTreePage(Size maxKeys, Flag unique = true)
        : m_maxKeys(maxKeys),
          m_maxKeysForChildren(maxKeys),
          m_unique(unique) {
        create();
    }

    virtual ~BTreePage() {
        reset();
    }

    Size keyCount() const {
        return m_keyCount;
    }

    Flag insert(const value_type& key, Ref ref, Flag& overflow) {
        Size pos = locate(key);
        overflow = false;

        if (pos < m_keyCount
            && equivalent(m_keys[pos].m_data, key)
            && m_unique) {
            return false;
        }

        if (!m_subPages[pos]) {
            insertAt(m_keys, Entry(key, ref), pos);
            ++m_keyCount;
            overflow = isOverflow();
            return true;
        }

        Flag childOverflow = false;
        if (!m_subPages[pos]->insert(key, ref, childOverflow))
            return false;

        if (childOverflow) {
            if (!redistribute(pos))
                splitChild(pos);
        }

        overflow = isOverflow();
        return true;
    }

    Flag search(
        const value_type& key,
        value_type& foundValue,
        Ref& foundRef) {
        const Size position = locate(key);

        if (position < m_keyCount
            && equivalent(m_keys[position].m_data, key)) {
            foundValue = m_keys[position].m_data;
            foundRef = m_keys[position].m_ref;
            m_keys[position].touch();
            return true;
        }

        return m_subPages[position]
            ? m_subPages[position]->search(key, foundValue, foundRef)
            : false;
    }

    template <typename Func, typename... Args>
    void forEach(Level level, Func& func, Args&&... args) {
        for (Size index = 0; index < m_keyCount; ++index) {
            if (m_subPages[index]) {
                m_subPages[index]->forEach(
                    level + 1,
                    func,
                    forward<Args>(args)...);
            }

            invoke(
                func,
                m_keys[index],
                level,
                forward<Args>(args)...);
        }

        if (m_subPages[m_keyCount]) {
            m_subPages[m_keyCount]->forEach(
                level + 1,
                func,
                forward<Args>(args)...);
        }
    }

    template <typename Func, typename... Args>
    Entry* firstThat(Level level, Func& func, Args&&... args) {
        for (Size index = 0; index < m_keyCount; ++index) {
            if (m_subPages[index]) {
                Entry* found = m_subPages[index]->firstThat(
                    level + 1,
                    func,
                    forward<Args>(args)...);
                if (found)
                    return found;
            }

            if (invoke(
                    func,
                    m_keys[index],
                    level,
                    forward<Args>(args)...)) {
                return &m_keys[index];
            }
        }

        return m_subPages[m_keyCount]
            ? m_subPages[m_keyCount]->firstThat(
                level + 1,
                func,
                forward<Args>(args)...)
            : nullptr;
    }

    template <typename Func, typename... Args>
    void forEachPage(Level level, Func& func, Args&&... args) {
        invoke(
            func,
            m_keyCount,
            level,
            forward<Args>(args)...);

        for (Size index = 0; index <= m_keyCount; ++index) {
            if (m_subPages[index]) {
                m_subPages[index]->forEachPage(
                    level + 1,
                    func,
                    forward<Args>(args)...);
            }
        }
    }
};

#endif // BTREE_PAGE_H
