// BTree.h

#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include "../types.h"
#include "traits.h"
#include "BTreePage.h"

// P1 Tarea Traits: template<typename Trait> en vez de template<typename keyType, typename ObjIDType>
template <typename Trait>
class BTree {
public:
    using value_type            = typename Trait::value_type;
    using Comp                  = typename Trait::Comp;
    static constexpr Size Order = Trait::Order;

    using Page  = BTreePage<Trait>;
    using Entry = typename Page::Entry;
    using Node  = Page;

    // P1 Tarea Adaptar Demo: iterador inorder con stack
    class Iterator {
        std::vector<std::pair<Page*, Size>> m_stack;
        const BTree* m_owner = nullptr;

        void pushPath(Page* page, Size idx) {
            while (page && page->m_keyCount > 0) {
                m_stack.push_back({page, idx});
                page = page->m_subPages[idx];
                idx = 0;
            }
        }

    public:
        Iterator() = default;
        explicit Iterator(Page* root, const BTree* owner) : m_owner(owner) { pushPath(root, 0); }

        Entry& operator*() const {
            std::shared_lock<std::shared_mutex> lock(m_owner->m_mtx);
            return m_stack.back().first->m_keys[m_stack.back().second];
        }

        Iterator& operator++() {
            std::shared_lock<std::shared_mutex> lock(m_owner->m_mtx);
            auto [page, idx] = m_stack.back();
            m_stack.pop_back();

            if (idx + 1 < page->m_keyCount)
                m_stack.push_back({page, idx + 1});

            Page* rightChild = page->m_subPages[idx + 1];
            if (rightChild) pushPath(rightChild, 0);

            return *this;
        }

        Flag operator==(const Iterator& other) const { return m_stack == other.m_stack; }
        Flag operator!=(const Iterator& other) const { return !(*this == other); }
    };

    Iterator begin()       { std::shared_lock<std::shared_mutex> lock(m_mtx); return Iterator(m_pRoot, this); }
    Iterator end()         { return Iterator(); }
    Iterator begin() const { std::shared_lock<std::shared_mutex> lock(m_mtx); return Iterator(m_pRoot, this); }
    Iterator end()   const { return Iterator(); }

private:
    Page*  m_pRoot;
    Depth  m_height;   // P1 Tarea Traits: size_t como indica la observacion
    Flag   m_unique;
    Size   m_numKeys;  // P1 Tarea Traits: size_t como indica la observacion
    mutable std::shared_mutex m_mtx;

    Page* deepCopy(Page* src) const {
        if (!src) return nullptr;
        auto* dst = new Page(src->m_maxKeys, src->m_unique);
        dst->m_maxKeysForChilds = src->m_maxKeysForChilds;
        dst->m_keyCount = src->m_keyCount;
        dst->m_keys     = src->m_keys;
        for (Size i = 0; i <= src->m_keyCount; ++i)
            dst->m_subPages[i] = deepCopy(src->m_subPages[i]);
        return dst;
    }

public:
    // Order viene del Trait en compile-time, ya no se pasa en runtime
    explicit BTree(Flag unique = true)
        : m_pRoot(new Page(2 * Order + 1, unique)), m_height(1), m_unique(unique), m_numKeys(0) {
        m_pRoot->setMaxKeysForChilds(Order);
    }

    // Copy constructor
    BTree(const BTree& other) : m_pRoot(nullptr), m_height(1), m_unique(true), m_numKeys(0) {
        std::shared_lock<std::shared_mutex> lockOther(other.m_mtx);
        m_pRoot   = deepCopy(other.m_pRoot);
        m_height  = other.m_height;
        m_unique  = other.m_unique;
        m_numKeys = other.m_numKeys;
    }

    // Move constructor
    BTree(BTree&& other) noexcept : m_pRoot(nullptr), m_height(1), m_unique(true), m_numKeys(0) {
        std::unique_lock<std::shared_mutex> lockOther(other.m_mtx);
        m_pRoot   = std::exchange(other.m_pRoot, nullptr);
        m_height  = std::exchange(other.m_height, 0);
        m_unique  = other.m_unique;
        m_numKeys = std::exchange(other.m_numKeys, 0);
    }

    // Copy assignment
    BTree& operator=(const BTree& other) {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lockThis(m_mtx);
            std::shared_lock<std::shared_mutex> lockOther(other.m_mtx);
            delete m_pRoot;
            m_pRoot   = deepCopy(other.m_pRoot);
            m_height  = other.m_height;
            m_unique  = other.m_unique;
            m_numKeys = other.m_numKeys;
        }
        return *this;
    }

    // Move assignment
    BTree& operator=(BTree&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lockThis(m_mtx), lockOther(other.m_mtx);
            delete m_pRoot;
            m_pRoot   = std::exchange(other.m_pRoot, nullptr);
            m_height  = std::exchange(other.m_height, 0);
            m_unique  = other.m_unique;
            m_numKeys = std::exchange(other.m_numKeys, 0);
        }
        return *this;
    }

    ~BTree() { delete m_pRoot; }

    Flag insert(const value_type& key, Ref ref) {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        auto error = m_pRoot->insert(key, ref);
        if (error == bt_ErrorCode::duplicate) return false;
        ++m_numKeys;
        if (error == bt_ErrorCode::overflow) { m_pRoot->splitRoot(); ++m_height; }
        return true;
    }

    std::tuple<value_type, Ref> remove(const value_type& key) {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        value_type outValue{}; Ref outRef{};
        auto error = m_pRoot->remove(key, outValue, outRef);
        if (error == bt_ErrorCode::notFound)
            throw std::runtime_error("BTree::remove - clave no encontrada");
        --m_numKeys;
        if (error == bt_ErrorCode::rootMerged) --m_height;
        return {outValue, outRef};
    }

    std::tuple<value_type, Ref> search(const value_type& key) const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        value_type outValue{}; Ref outRef{};
        if (!m_pRoot->search(key, outValue, outRef))
            throw std::runtime_error("BTree::search - clave no encontrada");
        return {outValue, outRef};
    }

    Size  size()   const { std::shared_lock<std::shared_mutex> lock(m_mtx); return m_numKeys; }
    Depth height() const { std::shared_lock<std::shared_mutex> lock(m_mtx); return m_height; }
    Size  order()  const { return Order; }

    // P1 Tarea ForEach Variadic: un solo forEach variadic que delega a la raiz
    template <typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        m_pRoot->forEach(0, func, std::forward<Args>(args)...);
    }

    // P1 Tarea ForEach Variadic: un solo firstThat variadic
    template <typename Func, typename... Args>
    Entry* firstThat(Func func, Args&&... args) {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return m_pRoot->firstThat(0, func, std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    void forEachPage(Func func, Args&&... args) {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        m_pRoot->forEachPage(0, func, std::forward<Args>(args)...);
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "[";
        Flag first = true;
        for (const auto& entry : *this) {
            if (!first) oss << ",";
            oss << "(" << entry << ")";
            first = false;
        }
        oss << "]";
        return oss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const BTree& tree) {
        return os << tree.toString();
    }

    friend std::istream& operator>>(std::istream& is, BTree& tree) {
        Token ch;
        if (!(is >> ch) || ch != '[') { is.clear(std::ios_base::failbit); return is; }
        Entry entry; Token paren;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> entry >> paren)
                    tree.insert(entry.m_data, entry.m_ref);
        return is;
    }
};

#endif // __BTREE_H__
