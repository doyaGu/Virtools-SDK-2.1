#ifndef XNHASHTABLE_H
#define XNHASHTABLE_H

#include "XArray.h"
#include "XHashFun.h"

#if VX_HAS_CXX11
#include <initializer_list>
#include <utility>
#endif

#ifdef VX_MSVC
#pragma warning(disable : 4786)
#endif

template <class T, class K, class H, class Eq>
class XNHashTable;

/**
 * @class XNHashTableEntry
 * @brief Represents a single entry (key-value pair) within a hash table bucket, allocated dynamically.
 *
 * @tparam T The type of the data stored.
 * @tparam K The type of the key used for hashing and lookup.
 *
 * @remarks This class is used internally by XNHashTable to manage collisions via chaining.
 * Each entry is allocated with 'new' and points to the next entry in the same bucket, forming a linked list.
 */
template <class T, class K>
class XNHashTableEntry
{
    typedef XNHashTableEntry<T, K> *tEntry;

public:
    /**
     * @brief Constructor to initialize with a key and a value.
     * @param k The key for the entry.
     * @param v The data for the entry.
     */
    XNHashTableEntry(const K &k, const T &v) : m_Key(k), m_Data(v), m_Next(0) {}

#if VX_HAS_CXX11
    /**
     * @brief Constructor to initialize with a key and a value by moving the value (C++11).
     */
    XNHashTableEntry(const K &k, T &&v) : m_Key(k), m_Data(std::move(v)), m_Next(0) {}

    /**
     * @brief Constructor to initialize with a key and a value by moving both (C++11).
     */
    XNHashTableEntry(K &&k, T &&v) : m_Key(std::move(k)), m_Data(std::move(v)), m_Next(0) {}
#endif

    /**
     * @brief Copy constructor.
     * @param e The entry to copy. The 'm_Next' pointer is initialized to null.
     */
    XNHashTableEntry(const XNHashTableEntry<T, K> &e) : m_Key(e.m_Key), m_Data(e.m_Data), m_Next(0) {}

    /**
     * @brief Destructor.
     */
    ~XNHashTableEntry() {}

    /// The key associated with this entry.
    K m_Key;
    /// The data stored in this entry.
    T m_Data;
    /// Pointer to the next entry in the same bucket (for collision resolution).
    tEntry m_Next;
};

/**
 * @class XNHashTableIt
 * @brief An iterator for traversing an XNHashTable.
 *
 * @tparam T The type of the value stored in the hash table.
 * @tparam K The type of the key used in the hash table.
 * @tparam H The hash function class.
 * @tparam Eq The equality comparison class for keys.
 *
 * @remarks
 * This iterator is the primary way to iterate over the elements in a hash table.
 * The iteration order is not guaranteed and will not necessarily match the insertion order.
 *
 * @example
 * @code
 * XNHashTableIt<MyType, MyKey, MyHash> it = hashtable.Begin();
 * while (it != hashtable.End()) {
 *     // access the key
 *     it.GetKey();
 *
 *     // access the element
 *     MyType& value = *it;
 *
 *     // move to the next element
 *     ++it;
 * }
 * @endcode
 */
template <class T, class K, class H = XHashFun<K>, class Eq = XEqual<K>>
class XNHashTableIt
{
    typedef XNHashTableEntry<T, K> *tEntry;
    typedef XNHashTableIt<T, K, H, Eq> tIterator;
    typedef XNHashTable<T, K, H, Eq> *tTable;
    friend class XNHashTable<T, K, H, Eq>;

public:
    /**
     * @brief Default constructor. Initializes a null iterator.
     */
    XNHashTableIt() : m_Node(0), m_Table(0) {}

    /**
     * @brief Copy constructor.
     * @param n The iterator to copy.
     */
    XNHashTableIt(const tIterator &n) : m_Node(n.m_Node), m_Table(n.m_Table) {}

    /**
     * @brief Equality operator.
     * @param it The iterator to compare against.
     * @return Non-zero if the iterators point to the same element, zero otherwise.
     */
    int operator==(const tIterator &it) const { return m_Node == it.m_Node; }

    /**
     * @brief Inequality operator.
     * @param it The iterator to compare against.
     * @return Non-zero if the iterators point to different elements, zero otherwise.
     */
    int operator!=(const tIterator &it) const { return m_Node != it.m_Node; }

    /**
     * @brief Dereference operator (const).
     * @return A constant reference to the data pointed to by the iterator.
     */
    const T &operator*() const { return (*m_Node).m_Data; }

    /**
     * @brief Dereference operator.
     * @return A reference to the data pointed to by the iterator, allowing modification.
     */
    T &operator*() { return (*m_Node).m_Data; }

    /**
     * @brief Conversion to a const pointer to the data.
     * @return A const pointer to the T object.
     */
    operator const T *() const { return &(m_Node->m_Data); }

    /**
     * @brief Conversion to a pointer to the data.
     * @return A pointer to the T object.
     */
    operator T *() { return &(m_Node->m_Data); }

    /**
     * @brief Gets the key of the entry pointed to by the iterator.
     * @return A constant reference to the key.
     */
    const K &GetKey() const { return m_Node->m_Key; }

    /**
     * @brief Pre-increment operator. Advances the iterator to the next element.
     * @return A reference to the incremented iterator.
     */
    tIterator &operator++()
    {
        // Prefixe
        tEntry old = m_Node;
        // next element of the linked list
        m_Node = m_Node->m_Next;

        if (!m_Node)
        {
            // end of linked list, we have to find next filled bucket
            // OPTIM : maybe keep the index current : save a %
            int index = m_Table->Index(old->m_Key);
            while (!m_Node && ++index < m_Table->m_Table.Size())
                m_Node = m_Table->m_Table[index];
        }
        return *this;
    }

    /**
     * @brief Post-increment operator. Advances the iterator to the next element.
     * @return A copy of the iterator before it was incremented.
     */
    tIterator operator++(int)
    {
        tIterator tmp = *this;
        ++*this;
        return tmp;
    }

    /**
     * @brief Internal constructor used by XNHashTable.
     * @param n Pointer to the hash table entry.
     * @param t Pointer to the hash table container.
     */
    XNHashTableIt(tEntry n, tTable t) : m_Node(n), m_Table(t) {}

    /// Pointer to the current hash table entry.
    tEntry m_Node;
    /// Pointer to the hash table this iterator belongs to.
    tTable m_Table;
};

/**
 * @class XNHashTableConstIt
 * @brief A constant iterator for traversing an XNHashTable.
 *
 * @tparam T The type of the value stored in the hash table.
 * @tparam K The type of the key used in the hash table.
 * @tparam H The hash function class.
 * @tparam Eq The equality comparison class for keys.
 *
 * @remarks
 * This iterator is used for traversing a `const` hash table. It does not allow modification
 * of the elements. The iteration order is not guaranteed.
 *
 * @example
 * @code
 * void MyClass::MyMethod() const
 * {
 *     XNHashTableConstIt<T,K,H> it = m_Hashtable.Begin();
 *     while (it != m_Hashtable.End()) {
 *         // access to the key
 *         it.GetKey();
 *
 *         // access to the element
 *         *it;
 *
 *         // next element
 *         ++it;
 *     }
 * }
 * @endcode
 */
template <class T, class K, class H = XHashFun<K>, class Eq = XEqual<K>>
class XNHashTableConstIt
{
    typedef XNHashTableEntry<T, K> *tEntry;
    typedef XNHashTableConstIt<T, K, H, Eq> tConstIterator;
    typedef XNHashTable<T, K, H, Eq> const *tConstTable;
    friend class XNHashTable<T, K, H, Eq>;

public:
    /**
     * @brief Default constructor. Initializes a null iterator.
     */
    XNHashTableConstIt() : m_Node(0), m_Table(0) {}

    /**
     * @brief Copy constructor.
     * @param n The iterator to copy.
     */
    XNHashTableConstIt(const tConstIterator &n) : m_Node(n.m_Node), m_Table(n.m_Table) {}

    /**
     * @brief Equality operator.
     * @param it The iterator to compare against.
     * @return Non-zero if the iterators point to the same element, zero otherwise.
     */
    int operator==(const tConstIterator &it) const { return m_Node == it.m_Node; }

    /**
     * @brief Inequality operator.
     * @param it The iterator to compare against.
     * @return Non-zero if the iterators point to different elements, zero otherwise.
     */
    int operator!=(const tConstIterator &it) const { return m_Node != it.m_Node; }

    /**
     * @brief Dereference operator.
     * @return A constant reference to the data pointed to by the iterator.
     */
    const T &operator*() const { return (*m_Node).m_Data; }

    /**
     * @brief Conversion to a const pointer to the data.
     * @return A const pointer to the T object.
     */
    operator const T *() const { return &(m_Node->m_Data); }

    /**
     * @brief Gets the key of the entry pointed to by the iterator.
     * @return A constant reference to the key.
     */
    const K &GetKey() const { return m_Node->m_Key; }

    /**
     * @brief Pre-increment operator. Advances the iterator to the next element.
     * @return A reference to the incremented iterator.
     */
    tConstIterator &operator++()
    {
        // Prefixe
        tEntry old = m_Node;
        // next element of the linked list
        m_Node = m_Node->m_Next;

        if (!m_Node)
        {
            // end of linked list, we have to find next filled bucket
            // OPTIM : maybe keep the index current : save a %
            int index = m_Table->Index(old->m_Key);
            while (!m_Node && ++index < m_Table->m_Table.Size())
                m_Node = m_Table->m_Table[index];
        }
        return *this;
    }

    /**
     * @brief Post-increment operator. Advances the iterator to the next element.
     * @return A copy of the iterator before it was incremented.
     */
    tConstIterator operator++(int)
    {
        tConstIterator tmp = *this;
        ++*this;
        return tmp;
    }

    /**
     * @brief Internal constructor used by XNHashTable.
     * @param n Pointer to the hash table entry.
     * @param t Pointer to the const hash table container.
     */
    XNHashTableConstIt(tEntry n, tConstTable t) : m_Node(n), m_Table(t) {}

    /// Pointer to the current hash table entry.
    tEntry m_Node;
    /// Pointer to the const hash table this iterator belongs to.
    tConstTable m_Table;
};

/**
 * @class XNHashTablePair
 * @brief A helper struct returned by TestInsert.
 *
 * @tparam T The type of the value stored in the hash table.
 * @tparam K The type of the key used in the hash table.
 * @tparam H The hash function class.
 * @tparam Eq The equality comparison class for keys.
 *
 * @remarks It contains an iterator to the element (either newly inserted or pre-existing)
 * and a boolean flag indicating whether the insertion was new.
 */
template <class T, class K, class H = XHashFun<K>, class Eq = XEqual<K>>
class XNHashTablePair
{
public:
    /**
     * @brief Constructor for the pair.
     * @param it Iterator to the element.
     * @param n A boolean value; TRUE if the element was newly inserted, FALSE if it already existed.
     */
    XNHashTablePair(XNHashTableIt<T, K, H, Eq> it, int n) : m_Iterator(it), m_New(n) {}

    /// An iterator pointing to the inserted or found element.
    XNHashTableIt<T, K, H, Eq> m_Iterator;
    /// A flag indicating if the element was newly inserted (TRUE) or already present (FALSE).
    XBOOL m_New;
};

/**
 * @class XNHashTable
 * @brief A container class implementing a hash table with dynamic node allocation.
 *
 * @tparam T The type of the element to insert.
 * @tparam K The type of the key.
 * @tparam H The hash function object used to hash the key. See XHashFun.h for default implementations.
 * @tparam Eq The equality comparison object for keys.
 *
 * @remarks
 * This implementation of the hash table uses a linked list in each bucket to resolve
 * hash collisions. Each entry (`XNHashTableEntry`) is allocated individually using `new`,
 * which means there are memory allocations for each insertion. For a static implementation
 * that uses a contiguous memory pool, see `XHashTable`.
 *
 * A `m_LoadFactor` member allows the user to decide at which occupation density the hash table
 * must be resized and rehashed.
 */
template <class T, class K, class H = XHashFun<K>, class Eq = XEqual<K>>
class XNHashTable
{
    // Types
    typedef XNHashTable<T, K, H, Eq> tTable;
    typedef XNHashTableEntry<T, K> *tEntry;
    typedef XNHashTableIt<T, K, H, Eq> tIterator;
    typedef XNHashTableConstIt<T, K, H, Eq> tConstIterator;
    typedef XNHashTablePair<T, K, H, Eq> tPair;

    /// @brief Friend class declaration for the iterator.
    friend class XNHashTableIt<T, K, H, Eq>;
    /// @brief Friend class declaration for the const iterator.
    friend class XNHashTableConstIt<T, K, H, Eq>;

public:
    /// @typedef Pair The type returned by TestInsert, containing an iterator and a boolean.
    typedef XNHashTablePair<T, K, H, Eq> Pair;
    /// @typedef Iterator The mutable iterator for this hash table.
    typedef XNHashTableIt<T, K, H, Eq> Iterator;
    /// @typedef ConstIterator The constant iterator for this hash table.
    typedef XNHashTableConstIt<T, K, H, Eq> ConstIterator;

    /**
     * @brief Default constructor.
     * @param initialize The initial number of buckets (should be a power of 2, otherwise it will be adjusted).
     * @param l The load factor, which determines when the table is resized.
     */
    XNHashTable(int initialize = 16, float l = 0.75f)
    {
        int dec = -1;
        while (initialize)
        {
            initialize >>= 1;
            dec++;
        }
        if (dec > -1)
            initialize = 1 << dec;
        else
            initialize = 1; // No Zero size allowed

        m_Table.Resize(initialize);
        m_Table.Memset(0);

        if (l <= 0.0)
            l = 0.75f;

        m_LoadFactor = l;
        m_Count = 0;
        m_Threshold = (int)(m_Table.Size() * m_LoadFactor);
    }

    /**
     * @brief Copy constructor.
     * @param a The hash table to copy.
     */
    XNHashTable(const XNHashTable &a)
    {
        XCopy(a);
    }

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     */
    XNHashTable(XNHashTable &&a) VX_NOEXCEPT { XMove(std::move(a)); }

    /**
     * @brief Constructs the table from an initializer list of key/value pairs (C++11).
     */
    XNHashTable(std::initializer_list<std::pair<K, T>> init, int initialize = 16, float l = 0.75f)
        : XNHashTable(((int)init.size() * 2 > initialize) ? (int)init.size() * 2 : initialize, l)
    {
        for (const auto &kv : init)
        {
            Insert(kv.first, kv.second);
        }
    }
#endif

    /**
     * @brief Destructor.
     * @remarks Releases all elements contained in the hash table by deleting each entry.
     * If you are storing pointers to objects, you must first iterate on the table and call
     * `delete` on each pointer before the hash table is destroyed.
     */
    ~XNHashTable()
    {
        Clear();
    }

    /**
     * @brief Removes all elements from the table.
     * @remarks Deletes all allocated entries. The number of buckets remains the same.
     */
    void Clear()
    {
        for (tEntry *it = m_Table.Begin(); it != m_Table.End(); it++)
        {
            // we destroy the linked list
            if (*it)
            {
                XDeleteList(*it);
                *it = NULL;
            }
        }
        m_Count = 0;
    }

    /**
     * @brief Assignment operator.
     * @param a The hash table to copy from.
     * @return A reference to this hash table.
     * @remarks The content of this table is entirely overwritten by the given table.
     */
    tTable &operator=(const tTable &a)
    {
        if (this != &a)
        {
            // We clear the current table
            Clear();
            // we then copy the content of a
            XCopy(a);
        }

        return *this;
    }

#if VX_HAS_CXX11
    /**
     * @brief Move assignment operator (C++11).
     */
    tTable &operator=(tTable &&a) VX_NOEXCEPT
    {
        if (this != &a)
        {
            Clear();
            XMove(std::move(a));
        }
        return *this;
    }

    /**
     * @brief Assigns the table from an initializer list of key/value pairs (C++11).
     */
    tTable &operator=(std::initializer_list<std::pair<K, T>> init)
    {
        Clear();
        for (const auto &kv : init)
        {
            Insert(kv.first, kv.second);
        }
        return *this;
    }
#endif

    /**
     * @brief Inserts an element, with an option to override existing elements.
     * @param key The key of the element.
     * @param o The element to insert.
     * @param override If TRUE and the key already exists, the existing element's value is updated. If FALSE, the insertion is skipped.
     * @return TRUE if the element was inserted or updated, FALSE otherwise.
     */
    XBOOL Insert(const K &key, const T &o, XBOOL override)
    {
        int index = Index(key);

        // we look for existing key
        tEntry e = XFind(index, key);
        if (!e)
        {
            if (m_Count >= m_Threshold)
            {
                // Need Rehash
                Rehash(m_Table.Size() * 2);
                return Insert(key, o, override);
            }
            else
            {
                // No
                XInsert(index, key, o);
            }
        }
        else
        {
            if (!override)
                return FALSE;
            e->m_Data = o;
        }

        return TRUE;
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element by moving the value (C++11).
     */
    XBOOL Insert(const K &key, T &&o, XBOOL override)
    {
        Eq equalFunc;

        while (true)
        {
            int index = Index(key);

            for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
            {
                if (equalFunc(e->m_Key, key))
                {
                    if (!override)
                        return FALSE;
                    e->m_Data = std::move(o);
                    return TRUE;
                }
            }

            if (m_Count >= m_Threshold)
            {
                Rehash(m_Table.Size() * 2);
                continue;
            }

            XInsert(index, key, std::move(o));
            return TRUE;
        }
    }
#endif

    /**
     * @brief Inserts or updates an element.
     * @param key The key of the element.
     * @param o The element to insert.
     * @return An iterator to the newly inserted or updated element.
     * @remarks If the key already exists, its value is overwritten.
     */
    tIterator Insert(const K &key, const T &o)
    {
        int index = Index(key);
        Eq equalFunc;

        // we look for existing key
        for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
        {
            if (equalFunc(e->m_Key, key))
            {
                e->m_Data = o;
                return tIterator(e, this);
            }
        }

        if (m_Count >= m_Threshold) // Need Rehash
        {
            Rehash(m_Table.Size() * 2);
            return Insert(key, o);
        }
        else // No
        {
            return tIterator(XInsert(index, key, o), this);
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts or updates an element by moving the value (C++11).
     */
    tIterator Insert(const K &key, T &&o)
    {
        Eq equalFunc;

        while (true)
        {
            int index = Index(key);
            for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
            {
                if (equalFunc(e->m_Key, key))
                {
                    e->m_Data = std::move(o);
                    return tIterator(e, this);
                }
            }

            if (m_Count >= m_Threshold)
            {
                Rehash(m_Table.Size() * 2);
                continue;
            }

            return tIterator(XInsert(index, key, std::move(o)), this);
        }
    }
#endif

    /**
     * @brief Inserts an element and reports whether it was new.
     * @param key The key of the element.
     * @param o The element to insert.
     * @return A `Pair` object containing an iterator to the element and a boolean.
     * The boolean is TRUE if the element was newly inserted, FALSE if the key already existed.
     */
    tPair TestInsert(const K &key, const T &o)
    {
        int index = Index(key);
        Eq equalFunc;

        // we look for existing key
        for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
        {
            if (equalFunc(e->m_Key, key))
            {
                return tPair(tIterator(e, this), 0);
            }
        }

        // Need Rehash
        if (m_Count >= m_Threshold)
        {
            // Yes
            Rehash(m_Table.Size() * 2);
            return TestInsert(key, o);
        }
        else
        {
            // No
            return tPair(tIterator(XInsert(index, key, o), this), 1);
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element and reports whether it was new by moving the value (C++11).
     */
    tPair TestInsert(const K &key, T &&o)
    {
        Eq equalFunc;

        while (true)
        {
            int index = Index(key);
            for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
            {
                if (equalFunc(e->m_Key, key))
                {
                    return tPair(tIterator(e, this), 0);
                }
            }

            if (m_Count >= m_Threshold)
            {
                Rehash(m_Table.Size() * 2);
                continue;
            }

            return tPair(tIterator(XInsert(index, key, std::move(o)), this), 1);
        }
    }
#endif

    /**
     * @brief Inserts an element only if the key does not already exist.
     * @param key The key of the element.
     * @param o The element to insert.
     * @return An iterator pointing to the element (either newly inserted or pre-existing).
     * @remarks This function will not overwrite an existing element.
     */
    tIterator InsertUnique(const K &key, const T &o)
    {
        int index = Index(key);
        Eq equalFunc;

        // we look for existing key
        for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
        {
            if (equalFunc(e->m_Key, key))
            {
                return tIterator(e, this);
            }
        }

        // Need Rehash
        if (m_Count >= m_Threshold)
        {
            // Yes
            Rehash(m_Table.Size() * 2);
            return InsertUnique(key, o);
        }
        else
        {
            // No
            tEntry newe = new XNHashTableEntry<T, K>(key, o);
            newe->m_Next = m_Table[index];
            m_Table[index] = newe;
            m_Count++;
            return tIterator(newe, this);
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element only if the key does not already exist by moving the value (C++11).
     */
    tIterator InsertUnique(const K &key, T &&o)
    {
        Eq equalFunc;

        while (true)
        {
            int index = Index(key);
            for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
            {
                if (equalFunc(e->m_Key, key))
                {
                    return tIterator(e, this);
                }
            }

            if (m_Count >= m_Threshold)
            {
                Rehash(m_Table.Size() * 2);
                continue;
            }

            tEntry newe = new XNHashTableEntry<T, K>(key, std::move(o));
            newe->m_Next = m_Table[index];
            m_Table[index] = newe;
            m_Count++;
            return tIterator(newe, this);
        }
    }

    /**
     * @brief Constructs a value in-place (via temporary) and inserts/overwrites (C++11).
     */
    template <class... Args>
    tIterator Emplace(const K &key, Args &&...args)
    {
        return Insert(key, T(std::forward<Args>(args)...));
    }

    /**
     * @brief Constructs a value in-place (via temporary) and inserts if missing (C++11).
     */
    template <class... Args>
    tIterator EmplaceUnique(const K &key, Args &&...args)
    {
        return InsertUnique(key, T(std::forward<Args>(args)...));
    }

    /**
     * @brief Constructs a value in-place (via temporary) and inserts if missing, reporting whether it was new (C++11).
     */
    template <class... Args>
    tPair TestEmplace(const K &key, Args &&...args)
    {
        return TestInsert(key, T(std::forward<Args>(args)...));
    }

    /// @brief STL-compatible begin/end for range-for and algorithms (C++11).
    tIterator begin() { return Begin(); }
    tConstIterator begin() const { return Begin(); }
    tConstIterator cbegin() const { return Begin(); }

    tIterator end() { return End(); }
    tConstIterator end() const { return End(); }
    tConstIterator cend() const { return End(); }
#endif

    /**
     * @brief Removes an element by its key.
     * @param key The key of the element to remove.
     */
    void Remove(const K &key)
    {
        int index = Index(key);
        Eq equalFunc;

        // we look for existing key
        tEntry old = NULL;
        for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
        {
            if (equalFunc(e->m_Key, key))
            {
                // This is the element to remove
                if (old)
                {
                    old->m_Next = e->m_Next;
                    delete e;
                }
                else
                {
                    m_Table[index] = e->m_Next;
                    delete e;
                }
                --m_Count;
                break;
            }
            old = e;
        }
    }

    /**
     * @brief Removes an element using an iterator.
     * @param it An iterator pointing to the element to remove.
     * @return An iterator to the element following the one that was removed.
     */
    tIterator Remove(const tIterator &it)
    {
        int index = Index(it.m_Node->m_Key);
        if (index >= m_Table.Size())
            return tIterator(0, this);

        // we look for existing key
        tEntry old = NULL;
        for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
        {
            if (e == it.m_Node)
            {
                // This is the element to remove
                if (old)
                {
                    old->m_Next = e->m_Next;
                    delete e;
                    old = old->m_Next;
                }
                else
                {
                    m_Table[index] = e->m_Next;
                    delete e;
                    old = m_Table[index];
                }
                --m_Count;
                break;
            }
            old = e;
        }
        // There is an element in the same column, we return it
        if (!old)
        {
            // No element in the same bucket, we parse for the next
            while (!old && ++index < m_Table.Size())
                old = m_Table[index];
        }

        return tIterator(old, this);
    }

    /**
     * @brief Accesses an element by key.
     * @param key The key of the element to access.
     * @return A reference to the element's value.
     * @remarks If no element corresponds to the key, a new element is created
     * with a default-constructed value (`T()`) and inserted into the table.
     */
    T &operator[](const K &key)
    {
        int index = Index(key);

        // we look for existing key
        tEntry e = XFind(index, key);
        if (!e)
        {
            if (m_Count >= m_Threshold)
            {
                // Need Rehash
                Rehash(m_Table.Size() * 2);
                return operator[](key);
            }
            else
            {
                // No
                e = XInsert(index, key, T());
            }
        }

        return e->m_Data;
    }

    /**
     * @brief Finds an element by key.
     * @param key The key of the element to find.
     * @return An `Iterator` to the found element, or `End()` if the element is not found.
     */
    tIterator Find(const K &key)
    {
        return tIterator(XFindIndex(key), this);
    }

    /**
     * @brief Finds an element by key in a constant hash table.
     * @param key The key of the element to find.
     * @return A `ConstIterator` to the found element, or `End()` if the element is not found.
     */
    tConstIterator Find(const K &key) const
    {
        return tConstIterator(XFindIndex(key), this);
    }

    /**
     * @brief Finds an element by key and returns a pointer to its value.
     * @param key The key of the element to find.
     * @return A pointer to the element's value if found, otherwise `NULL`.
     */
    T *FindPtr(const K &key) const
    {
        tEntry e = XFindIndex(key);
        if (e)
            return &e->m_Data;
        else
            return 0;
    }

    /**
     * @brief Searches for an element by key and retrieves its value.
     * @param key The key of the element to find.
     * @param[out] value A reference to a variable where the found value will be stored.
     * @return `TRUE` if the key was found, `FALSE` otherwise.
     */
    XBOOL LookUp(const K &key, T &value) const
    {
        tEntry e = XFindIndex(key);
        if (e)
        {
            value = e->m_Data;
            return TRUE;
        }
        else
            return FALSE;
    }

    /**
     * @brief Checks for the presence of a key in the hash table.
     * @param key The key to check for.
     * @return `TRUE` if the key was found, `FALSE` otherwise.
     */
    XBOOL IsHere(const K &key) const
    {
        return (XBOOL)XFindIndex(key);
    }

    /**
     * @brief Returns an iterator to the first element in the hash table.
     * @return An `Iterator` to the first element. If the table is empty, returns `End()`.
     *
     * @example
     * @code
     * XNHashTableIt<T,K,H> it = h.Begin();
     * XNHashTableIt<T,K,H> itend = h.End();
     *
     * for(; it != itend; ++it) {
     *     // do something with *it
     * }
     * @endcode
     */
    tIterator Begin()
    {
        for (tEntry *it = m_Table.Begin(); it != m_Table.End(); it++)
        {
            if (*it)
                return tIterator(*it, this);
        }
        return End();
    }

    /**
     * @brief Returns a constant iterator to the first element in the hash table.
     * @return A `ConstIterator` to the first element. If the table is empty, returns `End()`.
     */
    tConstIterator Begin() const
    {
        for (tEntry *it = m_Table.Begin(); it != m_Table.End(); it++)
        {
            if (*it)
                return tConstIterator(*it, this);
        }
        return End();
    }

    /**
     * @brief Returns an iterator pointing past the last element of the hash table.
     * @return An `Iterator` representing the end of the container.
     */
    tIterator End()
    {
        return tIterator(0, this);
    }

    /**
     * @brief Returns a constant iterator pointing past the last element of the hash table.
     * @return A `ConstIterator` representing the end of the container.
     */
    tConstIterator End() const
    {
        return tConstIterator(0, this);
    }

    /**
     * @brief Calculates the bucket index for a given key.
     * @param key The key to hash.
     * @return The index of the bucket in the hash table.
     */
    int Index(const K &key) const
    {
        H hashfun;
        return XIndex(hashfun(key), m_Table.Size());
    }

    /**
     * @brief Returns the number of elements in the hash table.
     * @return The total number of elements.
     */
    int Size() const
    {
        return m_Count;
    }

    /**
     * @brief Calculates the memory usage of the hash table.
     * @param addstatic If TRUE, includes the size of the `XNHashTable` object itself in the calculation.
     * @return The total memory occupied in bytes.
     */
    int GetMemoryOccupation(XBOOL addstatic = FALSE) const
    {
        return m_Table.GetMemoryOccupation() + m_Count * sizeof(XNHashTableEntry<T, K>) + (addstatic ? sizeof(*this) : 0);
    }

private:
    /**
     * @brief Gets a pointer to the start of the bucket array.
     * @return A pointer to the first bucket.
     */
    tEntry *GetFirstBucket() const { return m_Table.Begin(); }

    /**
     * @brief Resizes and rehashes the entire table.
     * @param size The new number of buckets for the table.
     */
    void
    Rehash(int size)
    {
        int oldsize = m_Table.Size();
        m_Threshold = (int)(size * m_LoadFactor);

        // Temporary table
        XArray<tEntry> tmp;
        tmp.Resize(size);
        tmp.Memset(0);

        for (int index = 0; index < oldsize; ++index)
        {
            tEntry first = m_Table[index];
            while (first)
            {
                H hashfun;
                int newindex = XIndex(hashfun(first->m_Key), size);
                m_Table[index] = first->m_Next;
                first->m_Next = tmp[newindex];
                tmp[newindex] = first;
                first = m_Table[index];
            }
        }
        m_Table.Swap(tmp);
    }

    /**
     * @brief Computes the final index from a hash key and table size.
     * @param key The hash value.
     * @param size The size of the table (must be a power of 2).
     * @return The bucket index.
     */
    int XIndex(int key, int size) const
    {
        return key & (size - 1);
    }

    /**
     * @brief Deletes all entries in a bucket's linked list.
     * @param e Pointer to the first entry in the list to delete.
     */
    void XDeleteList(tEntry e)
    {
        tEntry tmp = e;
        tEntry del;

        while (tmp != NULL)
        {
            del = tmp;
            tmp = tmp->m_Next;
            delete del;
        }
    }

    /**
     * @brief Performs a deep copy of a bucket's linked list.
     * @param e Pointer to the first entry of the list to copy.
     * @return A pointer to the first entry of the newly created list.
     */
    tEntry XCopyList(tEntry e)
    {
        tEntry tmp = e;
        tEntry newone;
        tEntry oldone = NULL;
        tEntry firstone = NULL;

        while (tmp != NULL)
        {
            newone = new XNHashTableEntry<T, K>(*tmp);
            if (oldone)
                oldone->m_Next = newone;
            else
                firstone = newone;
            oldone = newone;
            tmp = tmp->m_Next;
        }

        return firstone;
    }

    /**
     * @brief Internal helper to copy data from another hash table.
     * @param a The source hash table to copy from.
     */
    void XCopy(const XNHashTable &a)
    {
        m_Table.Resize(a.m_Table.Size());
        m_LoadFactor = a.m_LoadFactor;
        m_Count = a.m_Count;
        m_Threshold = a.m_Threshold;

        tEntry *it2 = a.GetFirstBucket();
        for (tEntry *it = m_Table.Begin(); it != m_Table.End(); ++it, ++it2)
        {
            *it = XCopyList(*it2);
        }
    }

    /**
     * @brief Internal helper to find an entry by key.
     * @param key The key to find.
     * @return A pointer to the found entry, or NULL if not found.
     */
    tEntry XFindIndex(const K &key) const
    {
        int index = Index(key);
        return XFind(index, key);
    }

    /**
     * @brief Internal helper to find an entry in a specific bucket.
     * @param index The bucket index to search in.
     * @param key The key to find.
     * @return A pointer to the found entry, or NULL if not found.
     */
    tEntry XFind(int index, const K &key) const
    {
        Eq equalFunc;

        // we look for existing key
        for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
        {
            if (equalFunc(e->m_Key, key))
            {
                return e;
            }
        }
        return NULL;
    }

    /**
     * @brief Internal helper to insert a new entry.
     * @param index The bucket index to insert into.
     * @param key The key of the new entry.
     * @param o The value of the new entry.
     * @return A pointer to the newly created entry.
     */
    tEntry XInsert(int index, const K &key, const T &o)
    {
        tEntry newe = new XNHashTableEntry<T, K>(key, o);
        newe->m_Next = m_Table[index];
        m_Table[index] = newe;
        ++m_Count;
        return newe;
    }

#if VX_HAS_CXX11
    tEntry XInsert(int index, const K &key, T &&o)
    {
        tEntry newe = new XNHashTableEntry<T, K>(key, std::move(o));
        newe->m_Next = m_Table[index];
        m_Table[index] = newe;
        ++m_Count;
        return newe;
    }
#endif

#if VX_HAS_CXX11
    void XMove(XNHashTable &&a)
    {
        m_Table = std::move(a.m_Table);
        m_Count = a.m_Count;
        m_Threshold = a.m_Threshold;
        m_LoadFactor = a.m_LoadFactor;

        a.m_Table.Resize(0);
        a.m_Count = 0;
        a.m_Threshold = 0;
        a.m_LoadFactor = 0.0f;
    }
#endif

    /// @brief The array of buckets, where each bucket is a pointer to the first entry in a linked list.
    XArray<tEntry> m_Table;
    /// @brief The total number of entries in the hash table.
    int m_Count;
    /// @brief The threshold at which the table will be rehashed (m_Table.Size() * m_LoadFactor).
    int m_Threshold;
    /// @brief The load factor for the hashtable.
    float m_LoadFactor;
};

#endif // XNHASHTABLE_H
