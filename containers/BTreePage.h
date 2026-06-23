// BTreePage.h

#ifndef __BTREEPAGE_H__
#define __BTREEPAGE_H__

#include <vector>
#include <utility>
#include "../types.h"
#include "traits.h"

template <typename Trait> class BTree;

// P1 Tarea Traits: enum class en vez de enum C
enum class bt_ErrorCode { ok, overflow, underflow, duplicate, notFound, rootMerged };

// P1 Tarea Traits: BTreeEntry reemplaza tagObjectInfo, ObjID ahora es Ref
template <typename Value>
struct BTreeEntry {
    using value_type = Value;

    Value m_data;
    Ref   m_ref;
    Size  m_useCount;

    BTreeEntry() : m_data(Value{}), m_ref(Ref{}), m_useCount(0) {}
    BTreeEntry(const Value& data, Ref ref) : m_data(data), m_ref(ref), m_useCount(0) {}

    operator const Value&() const { return m_data; }
    Size touch()          { return ++m_useCount; }
    Size useCount() const { return m_useCount; }

    Flag operator< (const BTreeEntry& other) const { return m_data <  other.m_data; }
    Flag operator> (const BTreeEntry& other) const { return m_data >  other.m_data; }
    Flag operator==(const BTreeEntry& other) const { return m_data == other.m_data; }

    friend std::ostream& operator<<(std::ostream& os, const BTreeEntry& entry) {
        return os << entry.m_data << ":" << entry.m_ref;
    }
    friend std::istream& operator>>(std::istream& is, BTreeEntry& entry) {
        Token sep;
        return is >> entry.m_data >> sep >> entry.m_ref;
    }
};

// P1 Tarea Traits: BTreePage ahora recibe un solo template param Trait
template <typename Trait>
class BTreePage {
public:
    using value_type            = typename Trait::value_type;
    using Comp                  = typename Trait::Comp;
    static constexpr Size Order = Trait::Order;
    using Entry                 = BTreeEntry<value_type>;
    using Page                  = BTreePage<Trait>;

private:
    friend class BTree<Trait>;
    Comp m_comp{};

    std::vector<Entry> m_keys;
    std::vector<Page*> m_subPages;
    Size m_keyCount;
    Size m_maxKeys;
    Size m_maxKeysForChilds;
    Flag m_unique;

    // locate reemplaza binary_search global
    // Si no lo encuentra, retorna la posicion donde deberia estar
    Size locate(const value_type& key) const {
        Size first = 0, last = m_keyCount;
        while (first < last) {
            Size mid = (first + last) / 2;
            if (key == m_keys[mid].m_data) return mid;
            if (key >  m_keys[mid].m_data) first = mid + 1;
            else                           last  = mid;
        }
        if (first <= m_keyCount && first < m_keys.size() && key <= m_keys[first].m_data)
            return first;
        return last;
    }

    // insertAt y removeAt reemplazan las funciones globales insert_at y remove
    template <typename Container, typename Item>
    static void insertAt(Container& c, const Item& item, Size pos) {
        Index n = c.size();
        for (Index i = n - 2; i >= (Index)pos; --i)
            c[i + 1] = c[i];
        c[pos] = item;
    }
    template <typename Container>
    static void removeAt(Container& c, Size pos) {
        Size n = c.size();
        for (Size i = pos + 1; i < n; ++i)
            c[i - 1] = c[i];
    }

    Size freeCells()   const { return m_maxKeys - m_keyCount; }
    Flag isFull()      const { return m_keyCount >= m_maxKeys; }
    Flag isOverflow()  const { return m_keyCount > m_maxKeys; }
    Size minKeys()     const { return 2 * m_maxKeys / 3; }
    Flag isUnderflow() const { return m_keyCount < minKeys(); }
    Flag isRoot()      const { return m_maxKeysForChilds != m_maxKeys; }
    Size freeCellsOnLeft(Size pos)  const { return pos > 0          ? m_subPages[pos-1]->freeCells() : 0; }
    Size freeCellsOnRight(Size pos) const { return pos < m_keyCount ? m_subPages[pos+1]->freeCells() : 0; }

    void setMaxKeysForChilds(Size order) { m_maxKeysForChilds = order; }

    void create() {
        m_keys.assign(m_maxKeys + 1, Entry{});
        m_subPages.assign(m_maxKeys + 2, nullptr);
        m_keyCount = 0;
    }
    void clearKeys() { m_keyCount = 0; }

    // Fix: el original usaba i < m_KeyCount, omitiendo el hijo mas derecho (m_subPages[m_keyCount])
    void reset() {
        for (Size i = 0; i <= m_keyCount; ++i) delete m_subPages[i];
        m_subPages.assign(m_subPages.size(), nullptr);
        clearKeys();
    }
    void destroy() { reset(); delete this; }

    Entry& firstEntry() {
        return m_subPages[0] ? m_subPages[0]->firstEntry() : m_keys[0];
    }

    Flag redistribute1(Size& pos) {
        if (m_subPages[pos]->isUnderflow()) {
            // nkLeft = Number of keys on left brother, nkRight = Number of keys on right brother
            Size nkLeft  = pos > 0          ? m_subPages[pos-1]->m_keyCount : 0;
            Size nkRight = pos < m_keyCount ? m_subPages[pos+1]->m_keyCount : 0;

            if (nkLeft > nkRight) {
                if (m_subPages[pos-1]->m_keyCount > m_subPages[pos-1]->minKeys())
                    redistributeL2R(pos - 1); // bring elements from left brother
                else if (pos == m_keyCount) { --pos; return false; }
                else return false;
            } else {
                if (m_subPages[pos+1]->m_keyCount > m_subPages[pos+1]->minKeys())
                    redistributeR2L(pos + 1); // bring elements from right brother
                else if (pos == 0) { ++pos; return false; }
                else return false;
            }
        } else {
            // it is due to overflow
            Size fcLeft = freeCellsOnLeft(pos), fcRight = freeCellsOnRight(pos);
            if (!fcLeft && !fcRight && m_subPages[pos]->isFull()) return false;
            if (fcLeft > fcRight) redistributeR2L(pos); else redistributeL2R(pos);
        }
        return true;
    }

    // Redistribute2 function
    // it considers two brothers m_SubPages[pos-1] && m_SubPages[pos+1]
    // if it fails the only way is merge
    Flag redistribute2(Size pos) {
        if (m_subPages[pos-1]->isUnderflow()) {
            redistributeR2L(pos + 1); redistributeR2L(pos);
            if (m_subPages[pos-1]->isUnderflow()) return false;
        } else if (m_subPages[pos+1]->isUnderflow()) {
            redistributeL2R(pos - 1); redistributeL2R(pos);
            if (m_subPages[pos+1]->isUnderflow()) return false;
        } else {
            // The problem is exactly at pos
            redistributeL2R(pos - 1); redistributeR2L(pos + 1);
            if (m_subPages[pos]->isUnderflow()) return false;
        }
        return true;
    }

    Flag treatUnderflow(Size& pos) { return redistribute1(pos) || redistribute2(pos); }

    void redistributeR2L(Size pos) {
        Page *src = m_subPages[pos], *dst = m_subPages[pos-1];
        while (src->m_keyCount > src->minKeys() && dst->m_keyCount < src->m_keyCount) {
            // Move from this page to the down-left page
            insertAt(dst->m_keys, m_keys[pos-1], dst->m_keyCount++);
            // Move the pointer leftest pointer to the rightest position
            insertAt(dst->m_subPages, src->m_subPages[0], dst->m_keyCount);
            // Move the leftest element to the root
            m_keys[pos-1] = src->m_keys[0];
            // Remove the leftest element from right page
            removeAt(src->m_keys, 0);
            removeAt(src->m_subPages, 0);
            --src->m_keyCount;
        }
    }

    void redistributeL2R(Size pos) {
        Page *src = m_subPages[pos], *dst = m_subPages[pos+1];
        while (src->m_keyCount > src->minKeys() && dst->m_keyCount < src->m_keyCount) {
            // Move from this page to the down-RIGHT page
            insertAt(dst->m_keys, m_keys[pos], 0);
            // Move the pointer rightest pointer to the leftest position
            insertAt(dst->m_subPages, src->m_subPages[src->m_keyCount], 0);
            ++dst->m_keyCount;
            // Move the rightest element to the root
            m_keys[pos] = src->m_keys[src->m_keyCount - 1];
            // it is not necessary erase because m_keyCount controls
            --src->m_keyCount;
        }
    }

    void movePage(Page* child, std::vector<Entry>& tmpKeys, std::vector<Page*>& tmpSub) {
        Size n = child->m_keyCount, i = 0;
        for (; i < n; ++i) { tmpKeys.push_back(child->m_keys[i]); tmpSub.push_back(child->m_subPages[i]); }
        tmpSub.push_back(child->m_subPages[i]);
        child->clearKeys();
    }

    void splitInto3(std::vector<Entry>& tmpKeys, std::vector<Page*>& tmpSub,
                    Page*& c1, Page*& c2, Page*& c3, Entry& e1, Entry& e2) {
        // Split tmpKeys page into 3 pages
        if (!c1) c1 = new Page(m_maxKeysForChilds, m_unique);
        // copy 1/3 elements to the first child
        c1->clearKeys();
        Size nKeys = (tmpKeys.size() - 2) / 3;
        Size i = 0;
        for (; i < nKeys; ++i) { c1->m_keys[i] = tmpKeys[i]; c1->m_subPages[i] = tmpSub[i]; ++c1->m_keyCount; }
        c1->m_subPages[i] = tmpSub[i];
        // first element to go up
        e1 = tmpKeys[i++];

        if (!c2) c2 = new Page(m_maxKeysForChilds, m_unique);
        c2->clearKeys();
        // copy 1/3 to the second child
        nKeys += (tmpKeys.size() - 2) / 3 + 1;
        Size j = 0;
        for (; i < nKeys; ++i, ++j) { c2->m_keys[j] = tmpKeys[i]; c2->m_subPages[j] = tmpSub[i]; ++c2->m_keyCount; }
        c2->m_subPages[j] = tmpSub[i];
        // copy the second element to the root
        e2 = tmpKeys[i++];

        // copy 1/3 to the third child
        if (!c3) c3 = new Page(m_maxKeysForChilds, m_unique);
        c3->clearKeys();
        nKeys = tmpKeys.size();
        for (j = 0; i < nKeys; ++i, ++j) { c3->m_keys[j] = tmpKeys[i]; c3->m_subPages[j] = tmpSub[i]; ++c3->m_keyCount; }
        c3->m_subPages[j] = tmpSub[i];
    }

    void splitChild(Size pos) {
        // FIRST: deciding the second page to split
        Page *c1 = nullptr, *c2 = nullptr;
        if (pos > 0 && m_subPages[pos-1]->isFull()) { c1 = m_subPages[pos-1]; c2 = m_subPages[pos--]; }
        if (pos < m_keyCount && m_subPages[pos+1]->isFull()) { c1 = m_subPages[pos]; c2 = m_subPages[pos+1]; }

        // SECOND: copy both pages to a temporal one
        std::vector<Entry> tmpKeys; std::vector<Page*> tmpSub;
        // Prepara el vector unificado de las 2 paginas a ser divididas en 3
        movePage(c1, tmpKeys, tmpSub);
        tmpKeys.push_back(m_keys[pos]);
        movePage(c2, tmpKeys, tmpSub);

        Page* c3 = nullptr; Entry e1, e2;
        splitInto3(tmpKeys, tmpSub, c1, c2, c3, e1, e2);

        // copy the first element to the root
        m_keys[pos] = e1; m_subPages[pos] = c1;
        // copy the second element to the root
        insertAt(m_keys, e2, pos + 1);
        insertAt(m_subPages, c2, pos + 1);
        ++m_keyCount;
        m_subPages[pos + 2] = c3;
    }

    Flag splitRoot() {
        Page *c1 = nullptr, *c2 = nullptr, *c3 = nullptr; Entry e1, e2;
        splitInto3(m_keys, m_subPages, c1, c2, c3, e1, e2);
        clearKeys();
        // copy the first element to the root
        m_keys[0] = e1; m_subPages[0] = c1; ++m_keyCount;
        // copy the second element to the root
        m_keys[1] = e2; m_subPages[1] = c2; ++m_keyCount;
        m_subPages[2] = c3;
        return true;
    }

    bt_ErrorCode mergePages(Size pos) {
        // FIRST: Put all the elements into a vector
        std::vector<Entry> tmpKeys; std::vector<Page*> tmpSub;
        Page *c1 = m_subPages[pos-1], *c2 = m_subPages[pos], *c3 = m_subPages[pos+1];
        movePage(c1, tmpKeys, tmpSub); tmpKeys.push_back(m_keys[pos-1]);
        movePage(c2, tmpKeys, tmpSub); tmpKeys.push_back(m_keys[pos]);
        movePage(c3, tmpKeys, tmpSub);
        c3->destroy();

        // Move 1/2 elements to c1
        Size nKeys = c1->freeCells(), i = 0;
        for (; i < nKeys; ++i) { c1->m_keys[i] = tmpKeys[i]; c1->m_subPages[i] = tmpSub[i]; ++c1->m_keyCount; }
        c1->m_subPages[i] = tmpSub[i];
        m_keys[pos-1] = tmpKeys[i]; m_subPages[pos-1] = c1;
        removeAt(m_keys, pos); removeAt(m_subPages, pos);
        --m_keyCount;

        nKeys = c2->freeCells();
        Size j = ++i;
        for (i = 0; i < nKeys; ++i, ++j) { c2->m_keys[i] = tmpKeys[j]; c2->m_subPages[i] = tmpSub[j]; ++c2->m_keyCount; }
        c2->m_subPages[i] = tmpSub[j];
        m_subPages[pos] = c2;

        return isUnderflow() ? bt_ErrorCode::underflow : bt_ErrorCode::ok;
    }

    bt_ErrorCode mergeRoot() {
        Size pos = 1;
        Page *c1 = m_subPages[pos-1], *c2 = m_subPages[pos], *c3 = m_subPages[pos+1];
        Size nKeys = c1->m_keyCount + c2->m_keyCount + c3->m_keyCount + 2;

        // FIRST: Put all the elements into a vector
        std::vector<Entry> tmpKeys; std::vector<Page*> tmpSub;
        movePage(c1, tmpKeys, tmpSub); tmpKeys.push_back(m_keys[pos-1]);
        movePage(c2, tmpKeys, tmpSub); tmpKeys.push_back(m_keys[pos]);
        movePage(c3, tmpKeys, tmpSub);

        clearKeys();
        Size i = 0;
        for (; i < nKeys; ++i) { m_keys[i] = tmpKeys[i]; m_subPages[i] = tmpSub[i]; ++m_keyCount; }
        m_subPages[i] = tmpSub[i];

        c1->destroy(); c2->destroy(); c3->destroy();
        return bt_ErrorCode::rootMerged;
    }

public:
    BTreePage(Size maxKeys, Flag unique = true)
        : m_keyCount(0), m_maxKeys(maxKeys), m_maxKeysForChilds(maxKeys), m_unique(unique) {
        create();
    }
    ~BTreePage() { reset(); }

    Size keyCount() const { return m_keyCount; }

    bt_ErrorCode insert(const value_type& key, Ref ref) {
        Size pos = locate(key);
        if (pos < m_keyCount && m_keys[pos].m_data == key && m_unique)
            return bt_ErrorCode::duplicate; // this key is duplicate

        if (!m_subPages[pos]) {
            // this is a leaf
            insertAt(m_keys, Entry(key, ref), pos);
            ++m_keyCount;
            return isOverflow() ? bt_ErrorCode::overflow : bt_ErrorCode::ok;
        }
        // recursive insertion
        auto error = m_subPages[pos]->insert(key, ref);
        if (error == bt_ErrorCode::duplicate) return bt_ErrorCode::duplicate;
        if (error == bt_ErrorCode::overflow) {
            if (!redistribute1(pos)) splitChild(pos);
            return isOverflow() ? bt_ErrorCode::overflow : bt_ErrorCode::ok;
        }
        return isOverflow() ? bt_ErrorCode::overflow : bt_ErrorCode::ok;
    }

    bt_ErrorCode remove(const value_type& key, value_type& outValue, Ref& outRef) {
        bt_ErrorCode error = bt_ErrorCode::ok;
        Size pos = locate(key);

        if (pos < m_keyCount && m_keys[pos].m_data == key) {
            // We found it
            outValue = m_keys[pos].m_data;
            outRef   = m_keys[pos].m_ref;
            if (!m_subPages[pos + 1]) {
                // This is a leaf: FIRST CASE
                removeAt(m_keys, pos);
                --m_keyCount;
                return isUnderflow() ? bt_ErrorCode::underflow : bt_ErrorCode::ok;
            }
            // We FOUND IT BUT it is NOT a leaf: SECOND CASE
            // Get the first element from right branch
            Entry& rightFirst = m_subPages[pos + 1]->firstEntry();
            // change with a leaf
            std::swap(m_keys[pos], rightFirst);
            // Remove it from this leaf
            value_type discard{}; Ref discardRef{};
            error = m_subPages[++pos]->remove(key, discard, discardRef);
        } else if (pos == m_keyCount) {
            // it is not here, go by the last branch
            error = m_subPages[pos]->remove(key, outValue, outRef);
        } else if (key <= m_keys[pos].m_data) {
            // = is because identical keys are inserted on left (see Insert)
            if (m_subPages[pos]) error = m_subPages[pos]->remove(key, outValue, outRef);
            else return bt_ErrorCode::notFound;
        }

        if (error == bt_ErrorCode::underflow) {
            // THIRD CASE: After removing the element we have an underflow
            if (treatUnderflow(pos)) return bt_ErrorCode::ok;
            // FOURTH CASE: it was not possible to redistribute -> Merge
            if (isRoot() && m_keyCount == 2) return mergeRoot();
            return mergePages(pos);
        }
        return error;
    }

    Flag search(const value_type& key, value_type& outValue, Ref& outRef) {
        Size pos = locate(key);
        if (pos >= m_keyCount)
            return m_subPages[pos] ? m_subPages[pos]->search(key, outValue, outRef) : false;
        if (m_keys[pos].m_data == key) {
            outValue = m_keys[pos].m_data;
            outRef   = m_keys[pos].m_ref;
            m_keys[pos].touch();
            return true;
        }
        if (key < m_keys[pos].m_data && m_subPages[pos])
            return m_subPages[pos]->search(key, outValue, outRef);
        return false;
    }

    // P1 Tarea ForEach Variadic: un solo forEach con variadic templates y perfect forwarding
    template <typename Func, typename... Args>
    void forEach(Depth depth, Func func, Args&&... args) {
        for (Size i = 0; i < m_keyCount; ++i) {
            if (m_subPages[i]) m_subPages[i]->forEach(depth + 1, func, std::forward<Args>(args)...);
            func(m_keys[i], depth, std::forward<Args>(args)...);
        }
        if (m_subPages[m_keyCount])
            m_subPages[m_keyCount]->forEach(depth + 1, func, std::forward<Args>(args)...);
    }

    // P1 Tarea ForEach Variadic: un solo firstThat con variadic templates
    template <typename Func, typename... Args>
    Entry* firstThat(Depth depth, Func func, Args&&... args) {
        for (Size i = 0; i < m_keyCount; ++i) {
            if (m_subPages[i])
                if (Entry* found = m_subPages[i]->firstThat(depth + 1, func, std::forward<Args>(args)...))
                    return found;
            if (func(m_keys[i], depth, std::forward<Args>(args)...)) return &m_keys[i];
        }
        if (m_subPages[m_keyCount])
            return m_subPages[m_keyCount]->firstThat(depth + 1, func, std::forward<Args>(args)...);
        return nullptr;
    }

    // P1 Tarea ForEach Variadic: recorre nodos (no claves)
    template <typename Func, typename... Args>
    void forEachPage(Depth depth, Func func, Args&&... args) {
        func(m_keyCount, depth, std::forward<Args>(args)...);
        for (Size i = 0; i <= m_keyCount; ++i)
            if (m_subPages[i]) m_subPages[i]->forEachPage(depth + 1, func, std::forward<Args>(args)...);
    }
};

#endif // __BTREEPAGE_H__
