#ifndef BTREE_H
#define BTREE_H

#include <algorithm>
#include <iterator>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "../types.h"
#include "BTreePage.h"
#include "traits.h"

using namespace std;

template <typename Trait>
class BTree {
public:
    using value_type = typename Trait::value_type;
    using Comp = typename Trait::Comp;
    using Page = BTreePage<Trait>;
    using Entry = typename Page::Entry;
    using Node = Page;

    class Iterator {
    public:
        using iterator_category = forward_iterator_tag;
        using value_type = Entry;
        using difference_type = ptrdiff_t;
        using pointer = Entry*;
        using reference = Entry&;

    private:
        vector<pair<Page*, Size>> m_stack;
        const BTree* m_owner{nullptr};

        void pushPath(Page* page, Size index) {
            while (page && page->m_keyCount > 0) {
                m_stack.emplace_back(page, index);
                page = page->m_subPages[index];
                index = 0;
            }
        }

    public:
        Iterator() = default;

        Iterator(Page* root, const BTree* owner)
            : m_owner(owner) {
            pushPath(root, 0);
        }

        reference operator*() const {
            lock_guard lock(m_owner->m_mutex);
            const auto& [page, index] = m_stack.back();
            return page->m_keys[index];
        }

        pointer operator->() const {
            return &operator*();
        }

        Iterator& operator++() {
            lock_guard lock(m_owner->m_mutex);
            auto [page, index] = m_stack.back();
            m_stack.pop_back();

            if (index + 1 < page->m_keyCount)
                m_stack.emplace_back(page, index + 1);

            pushPath(page->m_subPages[index + 1], 0);
            return *this;
        }

        Iterator operator++(int) {
            Iterator previous = *this;
            ++(*this);
            return previous;
        }

        friend Flag operator==(
            const Iterator& left,
            const Iterator& right) {
            return left.m_stack == right.m_stack;
        }

        friend Flag operator!=(
            const Iterator& left,
            const Iterator& right) {
            return !(left == right);
        }
    };

    // (backward iterator)
    class BackwardIterator {
    public:
        using iterator_category = forward_iterator_tag;
        using value_type = Entry;
        using difference_type = ptrdiff_t;
        using pointer = Entry*;
        using reference = Entry&;

    private:
        vector<pair<Page*, Size>> m_stack;
        const BTree* m_owner{nullptr};

        void pushRightPath(Page* page) {
            while (page && page->m_keyCount > 0) {
                Size index = page->m_keyCount - 1;
                m_stack.emplace_back(page, index);
                page = page->m_subPages[index + 1];
            }
        }

    public:
        BackwardIterator() = default;

        BackwardIterator(Page* root, const BTree* owner)
            : m_owner(owner) {
            pushRightPath(root);
        }

        reference operator*() const {
            lock_guard lock(m_owner->m_mutex);
            const auto& [page, index] = m_stack.back();
            return page->m_keys[index];
        }

        pointer operator->() const {
            return &operator*();
        }

        BackwardIterator& operator++() {
            lock_guard lock(m_owner->m_mutex);
            auto [page, index] = m_stack.back();
            m_stack.pop_back();

            if (index > 0)
                m_stack.emplace_back(page, index - 1);

            pushRightPath(page->m_subPages[index]);
            return *this;
        }

        BackwardIterator operator++(int) {
            BackwardIterator previous = *this;
            ++(*this);
            return previous;
        }

        friend Flag operator==(
            const BackwardIterator& left,
            const BackwardIterator& right) {
            return left.m_stack == right.m_stack;
        }

        friend Flag operator!=(
            const BackwardIterator& left,
            const BackwardIterator& right) {
            return !(left == right);
        }
    };

private:
    Page* m_root;
    Level m_height;
    Flag m_unique;
    Size m_numKeys;
    // (orden flexible)
    Size m_order;
    mutable mutex m_mutex;

    Page* deepCopy(const Page* source) const {
        if (!source)
            return nullptr;

        Page* copy =
            new Page(source->m_maxKeys, source->m_unique);
        copy->m_maxKeysForChildren =
            source->m_maxKeysForChildren;
        copy->m_keyCount = source->m_keyCount;
        copy->m_keys = source->m_keys;

        for (Size i = 0; i <= source->m_keyCount; ++i)
            copy->m_subPages[i] = deepCopy(source->m_subPages[i]);
        return copy;
    }

    Flag equivalent(
        const value_type& left,
        const value_type& right) const {
        const Comp comp{};
        return !invoke(comp, left, right)
            && !invoke(comp, right, left);
    }

    void insertUnlocked(const Entry& entry) {
        Flag overflow = false;
        if (!m_root->insert(entry.m_data, entry.m_ref, overflow))
            return;

        ++m_numKeys;
        if (overflow) {
            m_root->splitRoot();
            ++m_height;
        }
    }

    void rebuild(const vector<Entry>& entries) {
        delete m_root;
        m_root = new Page(2 * m_order + 1, m_unique);
        m_root->setMaxKeysForChildren(m_order);
        m_height = 1;
        m_numKeys = 0;

        for (const Entry& entry : entries)
            insertUnlocked(entry);

        vector<Entry*> ptrs;
        auto savePtr = [](Entry& e, Level, vector<Entry*>& out) {
            out.push_back(&e);
        };
        m_root->forEach(0, savePtr, ptrs);

        Size n = min(entries.size(), ptrs.size());
        for (Size i = 0; i < n; ++i)
            ptrs[i]->m_useCount = entries[i].m_useCount;
    }

public:
    // (orden flexible)
    explicit BTree(Size order = 3, Flag unique = true)
        : m_root(nullptr),
          m_height(1),
          m_unique(unique),
          m_numKeys(0),
          m_order(order) {
        if (m_order < 2)
            throw invalid_argument("El orden del BTree debe ser al menos 2");

        m_root = new Page(2 * m_order + 1, unique);
        m_root->setMaxKeysForChildren(m_order);
    }

    BTree(const BTree& other)
        : m_root(nullptr),
          m_height(1),
          m_unique(true),
          m_numKeys(0),
          m_order(3) {
        lock_guard lock(other.m_mutex);
        m_root = deepCopy(other.m_root);
        m_height = other.m_height;
        m_unique = other.m_unique;
        m_numKeys = other.m_numKeys;
        m_order = other.m_order;
    }

    BTree(BTree&& other) noexcept
        : m_root(nullptr),
          m_height(0),
          m_unique(true),
          m_numKeys(0),
          m_order(3) {
        unique_lock lock(other.m_mutex);
        m_root = exchange(other.m_root, nullptr);
        m_height = exchange(other.m_height, 0);
        m_unique = other.m_unique;
        m_numKeys = exchange(other.m_numKeys, 0);
        m_order = exchange(other.m_order, 3);
    }

    BTree& operator=(const BTree& other) {
        if (this == &other)
            return *this;

        BTree copy(other);
        unique_lock lock(m_mutex);
        swap(m_root, copy.m_root);
        swap(m_height, copy.m_height);
        swap(m_unique, copy.m_unique);
        swap(m_numKeys, copy.m_numKeys);
        swap(m_order, copy.m_order);
        return *this;
    }

    BTree& operator=(BTree&& other) noexcept {
        if (this == &other)
            return *this;

        scoped_lock lock(m_mutex, other.m_mutex);
        delete m_root;
        m_root = exchange(other.m_root, nullptr);
        m_height = exchange(other.m_height, 0);
        m_unique = other.m_unique;
        m_numKeys = exchange(other.m_numKeys, 0);
        m_order = exchange(other.m_order, 3);
        return *this;
    }

    virtual ~BTree() {
        delete m_root;
    }

    Flag insert(const value_type& key, Ref ref) {
        unique_lock lock(m_mutex);
        if (!m_root) {
            m_root = new Page(2 * m_order + 1, m_unique);
            m_root->setMaxKeysForChildren(m_order);
            m_height = 1;
        }

        Flag overflow = false;
        if (!m_root->insert(key, ref, overflow))
            return false;

        ++m_numKeys;
        if (overflow) {
            m_root->splitRoot();
            ++m_height;
        }
        return true;
    }

    tuple<value_type, Ref> remove(const value_type& key) {
        unique_lock lock(m_mutex);
        if (!m_root)
            throw runtime_error(
                "BTree::remove - clave no encontrada");

        vector<Entry> entries;
        auto collect = [](Entry& e, Level, vector<Entry>& out) {
            out.push_back(e);
        };
        m_root->forEach(0, collect, entries);

        const auto found = find_if(
            entries.begin(),
            entries.end(),
            [this, &key](const Entry& e) {
                return equivalent(e.m_data, key);
            });

        if (found == entries.end()) {
            throw runtime_error(
                "BTree::remove - clave no encontrada");
        }

        value_type value = found->m_data;
        Ref ref = found->m_ref;
        entries.erase(found);
        rebuild(entries);

        return {value, ref};
    }

    tuple<value_type, Ref> search(const value_type& key) const {
        unique_lock lock(m_mutex);
        if (!m_root)
            throw runtime_error(
                "BTree::search - clave no encontrada");

        value_type value{};
        Ref ref{};
        if (!m_root->search(key, value, ref)) {
            throw runtime_error(
                "BTree::search - clave no encontrada");
        }
        return {value, ref};
    }

    Size size() const {
        lock_guard lock(m_mutex);
        return m_numKeys;
    }

    Level height() const {
        lock_guard lock(m_mutex);
        return m_height;
    }

    Size order() const {
        lock_guard lock(m_mutex);
        return m_order;
    }

    template <typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        lock_guard lock(m_mutex);
        if (m_root) {
            m_root->forEach(
                0,
                func,
                forward<Args>(args)...);
        }
    }

    template <typename Func, typename... Args>
    Entry* firstThat(Func func, Args&&... args) {
        lock_guard lock(m_mutex);
        return m_root
            ? m_root->firstThat(
                0,
                func,
                forward<Args>(args)...)
            : nullptr;
    }

    Iterator begin() {
        lock_guard lock(m_mutex);
        return Iterator(m_root, this);
    }

    Iterator end() {
        return Iterator();
    }

    Iterator begin() const {
        lock_guard lock(m_mutex);
        return Iterator(m_root, this);
    }

    Iterator end() const {
        return Iterator();
    }

    // (backward iterator)
    BackwardIterator rbegin() {
        lock_guard lock(m_mutex);
        return BackwardIterator(m_root, this);
    }

    BackwardIterator rend() {
        return BackwardIterator();
    }

    BackwardIterator rbegin() const {
        lock_guard lock(m_mutex);
        return BackwardIterator(m_root, this);
    }

    BackwardIterator rend() const {
        return BackwardIterator();
    }

    string toString() const {
        ostringstream output;
        output << '[';
        Flag first = true;

        for (const Entry& entry : *this) {
            if (!first)
                output << ',';
            output << '(' << entry << ')';
            first = false;
        }

        output << ']';
        return output.str();
    }

    friend ostream& operator<<(
        ostream& os,
        const BTree& tree) {
        return os << tree.toString();
    }

    friend istream& operator>>(
        istream& is,
        BTree& tree) {
        Token token{};
        if (!(is >> token) || token != '[') {
            is.setstate(ios_base::failbit);
            return is;
        }

        while (is >> token) {
            if (token == ']')
                return is;

            if (token != '(')
                continue;

            Entry entry;
            Token closingParenthesis{};
            if (!(is >> entry >> closingParenthesis)
                || closingParenthesis != ')') {
                is.setstate(ios_base::failbit);
                return is;
            }
            tree.insert(entry.m_data, entry.m_ref);
        }

        is.setstate(ios_base::failbit);
        return is;
    }
};

#endif // BTREE_H
