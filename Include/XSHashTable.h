/**
 * @file XSHashTable.h
 * @brief This file defines the XSHashTable class, a static hash table implementation using open addressing.
 */

#ifndef XSHASHTABLE_H
#define XSHASHTABLE_H

#include "XArray.h"
#include "XClassArray.h"
#include "XHashFun.h"

#if VX_HAS_CXX11
#include <algorithm>
#include <initializer_list>
#include <utility>
#endif

template <class T, class K, class H, class Eq>
class XSHashTable;

/// @brief Indicates that a hash table entry is available.
#define STATUS_FREE 0
/// @brief Indicates that a hash table entry is occupied by an element.
#define STATUS_OCCUPIED 1
/// @brief Indicates that a hash table entry was occupied but its element has been deleted.
#define STATUS_DELETED 2

/**
 * @class XSHashTableEntry
 * @brief Represents a single entry in a static hash table.
 *
 * @tparam T The type of the data stored.
 * @tparam K The type of the key used for hashing and lookup.
 *
 * @remarks This class holds the key, data, and the status of the bucket (free, occupied, or deleted),
 * which is essential for open addressing collision resolution.
 */
template <class T, class K>
class XSHashTableEntry
{
    typedef XSHashTableEntry<T, K> *pEntry;

public:
    /**
     * @brief Default constructor. Initializes the entry status to FREE.
     */
    XSHashTableEntry() : m_Status(STATUS_FREE) {}

    /**
     * @brief Copy constructor. Initializes the entry with data from another entry and sets its status to OCCUPIED.
     * @param e The entry to copy.
     */
    XSHashTableEntry(const XSHashTableEntry<T, K> &e) : m_Key(e.m_Key), m_Data(e.m_Data), m_Status(STATUS_OCCUPIED) {}

    /**
     * @brief Copy assignment operator.
     * @param e The entry to copy from.
     * @return A reference to this entry.
     */
    XSHashTableEntry<T, K> &operator=(const XSHashTableEntry<T, K> &e)
    {
        if (this != &e)
        {
            m_Key = e.m_Key;
            m_Data = e.m_Data;
            m_Status = e.m_Status;
        }
        return *this;
    }

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @param e The entry to move from. The source entry's status is set to FREE.
     */
    XSHashTableEntry(XSHashTableEntry<T, K> &&e) VX_NOEXCEPT : m_Key(std::move(e.m_Key)), m_Data(std::move(e.m_Data)), m_Status(e.m_Status)
    {
        e.m_Status = STATUS_FREE;
    }

    /**
     * @brief Move assignment operator (C++11).
     * @param e The entry to move from.
     * @return A reference to this entry.
     */
    XSHashTableEntry<T, K> &operator=(XSHashTableEntry<T, K> &&e) VX_NOEXCEPT
    {
        if (this != &e)
        {
            m_Key = std::move(e.m_Key);
            m_Data = std::move(e.m_Data);
            m_Status = e.m_Status;
            e.m_Status = STATUS_FREE;
        }
        return *this;
    }
#endif
    /**
     * @brief Destructor.
     */
    ~XSHashTableEntry()
    {
    }

    /**
     * @brief Sets the key and data for this entry and marks it as occupied.
     * @param key The key to set.
     * @param data The data to set.
     */
    void Set(const K &key, const T &data)
    {
        m_Key = key;
        m_Data = data;
        m_Status = STATUS_OCCUPIED;
    }

#if VX_HAS_CXX11
    /**
     * @brief Sets the key and data for this entry and marks it as occupied by moving the data (C++11).
     */
    void Set(const K &key, T &&data)
    {
        m_Key = key;
        m_Data = std::move(data);
        m_Status = STATUS_OCCUPIED;
    }

    /**
     * @brief Sets the key and data for this entry and marks it as occupied by moving both (C++11).
     */
    void Set(K &&key, T &&data)
    {
        m_Key = std::move(key);
        m_Data = std::move(data);
        m_Status = STATUS_OCCUPIED;
    }
#endif

    /// The key associated with this entry.
    K m_Key;
    /// The data stored in this entry.
    T m_Data;
    /// The status of the entry (FREE, OCCUPIED, or DELETED).
    int m_Status;
};

/**
 * @class XSHashTableIt
 * @brief An iterator for traversing an XSHashTable.
 *
 * @tparam T The type of the value stored in the hash table.
 * @tparam K The type of the key used in the hash table.
 * @tparam H The hash function class.
 * @tparam Eq The equality comparison class for keys.
 *
 * @remarks
 * This iterator is the primary way to iterate over the elements in a static hash table.
 * The iteration order is not guaranteed and will not necessarily match the insertion order.
 *
 * @example
 * @code
 * XSHashTableIt<MyType, MyKey, MyHash> it = hashtable.Begin();
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
class XSHashTableIt
{
    typedef XSHashTableEntry<T, K> *pEntry;
    typedef XSHashTableIt<T, K, H, Eq> tIterator;
    typedef XSHashTable<T, K, H, Eq> *tTable;
    friend class XSHashTable<T, K, H, Eq>;

public:
    /**
     * @brief Default constructor. Initializes a null iterator.
     */
    XSHashTableIt() : m_Node(0), m_Table(0) {}

    /**
     * @brief Copy constructor.
     * @param n The iterator to copy.
     */
    XSHashTableIt(const tIterator &n) : m_Node(n.m_Node), m_Table(n.m_Table) {}

    /**
     * @brief Copy assignment operator.
     * @param n The iterator to copy from.
     * @return A reference to this iterator.
     */
    tIterator &operator=(const tIterator &n)
    {
        if (this != &n)
        {
            m_Node = n.m_Node;
            m_Table = n.m_Table;
        }
        return *this;
    }

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @param n The iterator to move from.
     */
    XSHashTableIt(tIterator &&n) VX_NOEXCEPT : m_Node(n.m_Node), m_Table(n.m_Table)
    {
        n.m_Node = NULL;
        n.m_Table = NULL;
    }

    /**
     * @brief Move assignment operator (C++11).
     * @param n The iterator to move from.
     * @return A reference to this iterator.
     */
    tIterator &operator=(tIterator &&n) VX_NOEXCEPT
    {
        if (this != &n)
        {
            m_Node = n.m_Node;
            m_Table = n.m_Table;
            n.m_Node = NULL;
            n.m_Table = NULL;
        }
        return *this;
    }
#endif

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
     * @brief Gets the key of the entry pointed to by the iterator.
     * @return A constant reference to the key.
     */
    const K &GetKey() const { return m_Node->m_Key; }

    /**
     * @brief Pre-increment operator. Advances the iterator to the next occupied element.
     * @return A reference to the incremented iterator.
     */
    tIterator &operator++()
    {
        // Prefixe
        ++m_Node;
        pEntry end = m_Table->m_Table.End();
        while (m_Node != end)
        {
            // we're not at the end of the list yet
            if (m_Node->m_Status == STATUS_OCCUPIED)
                break;
            ++m_Node;
        }
        return *this;
    }

    /**
     * @brief Post-increment operator. Advances the iterator to the next occupied element.
     * @return A copy of the iterator before it was incremented.
     */
    tIterator operator++(int)
    {
        tIterator tmp = *this;
        ++*this;
        return tmp;
    }

protected:
    /**
     * @brief Internal constructor used by XSHashTable.
     * @param n Pointer to the hash table entry.
     * @param t Pointer to the hash table container.
     */
    XSHashTableIt(pEntry n, tTable t) : m_Node(n), m_Table(t) {}

    /// @brief Pointer to the current hash table entry.
    pEntry m_Node;
    /// @brief Pointer to the hash table this iterator belongs to.
    tTable m_Table;
};

/**
 * @class XSHashTableConstIt
 * @brief A constant iterator for traversing an XSHashTable.
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
 *     XSHashTableConstIt<T,K,H> it = m_Hashtable.Begin();
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
class XSHashTableConstIt
{
    typedef XSHashTableEntry<T, K> *pEntry;
    typedef XSHashTableConstIt<T, K, H, Eq> tConstIterator;
    typedef XSHashTable<T, K, H, Eq> const *tConstTable;
    friend class XSHashTable<T, K, H, Eq>;

public:
    /**
     * @brief Default constructor. Initializes a null iterator.
     */
    XSHashTableConstIt() : m_Node(0), m_Table(0) {}

    /**
     * @brief Copy constructor.
     * @param n The iterator to copy.
     */
    XSHashTableConstIt(const tConstIterator &n) : m_Node(n.m_Node), m_Table(n.m_Table) {}

    /**
     * @brief Copy assignment operator.
     * @param n The iterator to copy from.
     * @return A reference to this iterator.
     */
    tConstIterator &operator=(const tConstIterator &n)
    {
        if (this != &n)
        {
            m_Node = n.m_Node;
            m_Table = n.m_Table;
        }
        return *this;
    }

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @param n The iterator to move from.
     */
    XSHashTableConstIt(tConstIterator &&n) VX_NOEXCEPT : m_Node(n.m_Node), m_Table(n.m_Table)
    {
        n.m_Node = NULL;
        n.m_Table = NULL;
    }

    /**
     * @brief Move assignment operator (C++11).
     * @param n The iterator to move from.
     * @return A reference to this iterator.
     */
    tConstIterator &operator=(tConstIterator &&n) VX_NOEXCEPT
    {
        if (this != &n)
        {
            m_Node = n.m_Node;
            m_Table = n.m_Table;
            n.m_Node = NULL;
            n.m_Table = NULL;
        }
        return *this;
    }
#endif

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
     * @brief Pre-increment operator. Advances the iterator to the next occupied element.
     * @return A reference to the incremented iterator.
     */
    tConstIterator &operator++()
    {
        // Prefixe
        ++m_Node;
        pEntry end = m_Table->m_Table.End();
        while (m_Node != end)
        {
            // we're not at the end of the list yet
            if (m_Node->m_Status == STATUS_OCCUPIED)
                break;
            ++m_Node;
        }
        return *this;
    }

    /**
     * @brief Post-increment operator. Advances the iterator to the next occupied element.
     * @return A copy of the iterator before it was incremented.
     */
    tConstIterator operator++(int)
    {
        tConstIterator tmp = *this;
        ++*this;
        return tmp;
    }

protected:
    /**
     * @brief Internal constructor used by XSHashTable.
     * @param n Pointer to the hash table entry.
     * @param t Pointer to the const hash table container.
     */
    XSHashTableConstIt(pEntry n, tConstTable t) : m_Node(n), m_Table(t) {}

    /// @brief Pointer to the current hash table entry.
    pEntry m_Node;
    /// @brief Pointer to the const hash table this iterator belongs to.
    tConstTable m_Table;
};

/**
 * @class XSHashTablePair
 * @brief A helper struct returned by some insertion methods.
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
class XSHashTablePair
{
public:
    /**
     * @brief Constructor for the pair.
     * @param it Iterator to the element.
     * @param n A boolean value; TRUE if the element was newly inserted, FALSE if it already existed.
     */
    XSHashTablePair(XSHashTableIt<T, K, H> it, int n) : m_Iterator(it), m_New(n) {}

    /// An iterator pointing to the inserted or found element.
    XSHashTableIt<T, K, H> m_Iterator;
    /// A flag indicating if the element was newly inserted (TRUE) or already present (FALSE).
    XBOOL m_New;
};

/**
 * @class XSHashTable
 * @brief A "static" hash table container that uses open addressing with linear probing.
 *
 * @tparam T The type of the element to insert.
 * @tparam K The type of the key.
 * @tparam H The hash function object used to hash the key. See XHashFun.h for default implementations.
 * @tparam Eq The equality comparison object for keys.
 *
 * @remarks
 * This implementation ("S" for "Static") uses a single contiguous block of memory (`XClassArray`)
 * for all entries. It resolves collisions using open addressing (specifically linear probing),
 * which avoids per-element dynamic memory allocations, contrasting with `XNHashTable`
 * and `XHashTable`.
 *
 * A `m_LoadFactor` member determines when the hash table should be extended and rehashed
 * to maintain performance.
 */
template <class T, class K, class H = XHashFun<K>, class Eq = XEqual<K>>
class XSHashTable
{
    // Types
    typedef XSHashTable<T, K, H, Eq> tTable;
    typedef XSHashTableEntry<T, K> tEntry;
    typedef tEntry *pEntry;
    typedef XSHashTableIt<T, K, H, Eq> tIterator;
    typedef XSHashTableConstIt<T, K, H, Eq> tConstIterator;
    typedef XSHashTablePair<T, K, H, Eq> tPair;

    /// @brief Friend class declaration for the iterator.
    friend class XSHashTableIt<T, K, H, Eq>;
    /// @brief Friend class declaration for the const iterator.
    friend class XSHashTableConstIt<T, K, H, Eq>;

public:
    /// @typedef Iterator The mutable iterator for this hash table.
    typedef XSHashTableIt<T, K, H, Eq> Iterator;
    /// @typedef ConstIterator The constant iterator for this hash table.
    typedef XSHashTableConstIt<T, K, H, Eq> ConstIterator;

#if VX_HAS_CXX11
    /// @typedef Pair The type returned by TestInsert/TestEmplace (C++11 convenience).
    typedef XSHashTablePair<T, K, H, Eq> Pair;
#endif

    /**
     * @brief Constructor.
     * @param initialize The initial number of buckets (will be rounded up to the next power of 2).
     * @param l The load factor, which determines when the table is resized.
     */
    explicit XSHashTable(int initialize = 8, float l = 0.75f)
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
            initialize = 1; // No 0 size allowed
        m_Table.Resize(initialize);

        if (l <= 0.0)
            l = 0.75f;

        m_LoadFactor = l;
        m_Count = 0;
        m_Occupation = 0;
        m_Threshold = (int)(m_Table.Size() * m_LoadFactor);
    }

    /**
     * @brief Copy constructor.
     * @param a The hash table to copy.
     */
    XSHashTable(const XSHashTable &a) { XCopy(a); }

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @param a The hash table to move from.
     */
    XSHashTable(XSHashTable &&a) VX_NOEXCEPT { XMove(std::move(a)); }

    /**
     * @brief Constructs the table from an initializer list of key/value pairs (C++11).
     * @remarks Existing entries are inserted using Insert(key, value, TRUE).
     */
    XSHashTable(std::initializer_list<std::pair<K, T>> init, int initialize = 8, float l = 0.75f)
        : XSHashTable((initialize > (int)init.size() * 2) ? initialize : (int)init.size() * 2, l)
    {
        for (const auto &kv : init)
        {
            Insert(kv.first, kv.second, TRUE);
        }
    }
#endif

    /**
     * @brief Destructor.
     * @remarks Releases the elements contained in the hash table. If you are storing pointers,
     * you must iterate over the table and manually delete each pointed-to object before destroying the table.
     */
    ~XSHashTable() {}

    /**
     * @brief Removes all elements from the table by marking them as FREE.
     * @remarks The hash table remains with the same number of buckets after a clear.
     */
    void Clear()
    {
        for (pEntry it = m_Table.Begin(); it != m_Table.End(); it++)
        {
            // we destroy the linked list
            (*it).m_Status = STATUS_FREE;
        }
        m_Count = 0;
        m_Occupation = 0;
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
     * @param a The hash table to move from.
     * @return A reference to this hash table.
     */
    tTable &operator=(tTable &&a) VX_NOEXCEPT
    {
        if (this != &a)
        {
            // We clear the current table
            Clear();
            // we then move the content of a
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
            Insert(kv.first, kv.second, TRUE);
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
        // Insert x as active
        int index = XFindPos(key);

        if (m_Table[index].m_Status != STATUS_OCCUPIED)
        {
            // If the element was deleted, we remove an element
            if ((m_Table[index].m_Status != STATUS_DELETED))
            {
                ++m_Occupation;
                ++m_Count;
            }
            else
            {
                ++m_Count;
            }
        }
        else
        {
            // Occupied
            if (override)
            {
                // no count or occupation change
            }
            else
                return FALSE;
        }
        m_Table[index].Set(key, o);

        // Test the rehash need
        if (m_Occupation < m_Threshold)
            return TRUE;

        Rehash(m_Table.Size() * 2);
        return TRUE;
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element by moving the value (C++11).
     */
    XBOOL Insert(const K &key, T &&o, XBOOL override)
    {
        int index = XFindPos(key);

        if (m_Table[index].m_Status != STATUS_OCCUPIED)
        {
            if ((m_Table[index].m_Status != STATUS_DELETED))
            {
                ++m_Occupation;
                ++m_Count;
            }
            else
            {
                ++m_Count;
            }
        }
        else
        {
            if (!override)
                return FALSE;
        }

        m_Table[index].Set(key, std::move(o));

        if (m_Occupation < m_Threshold)
            return TRUE;

        Rehash(m_Table.Size() * 2);
        return TRUE;
    }
#endif

    /**
     * @brief Inserts an element only if the key does not already exist.
     * @param key The key of the element.
     * @param o The element to insert.
     * @return An iterator pointing to the element (either newly inserted or pre-existing).
     */
    tIterator InsertUnique(const K &key, const T &o)
    {
        // Insert x as active
        int index = XFindPos(key);

        if (m_Table[index].m_Status != STATUS_OCCUPIED)
        {
            // If the element was deleted, we remove an element
            if ((m_Table[index].m_Status != STATUS_DELETED))
            {
                ++m_Occupation;
                ++m_Count;
            }
            else
            {
                ++m_Count;
            }
        }
        else // Occupied
        {
            return tIterator(&m_Table[index], this);
        }

        // Need Rehash
        if (m_Count >= m_Threshold) // Yes
        {
            Rehash(m_Table.Size() * 2);
            return InsertUnique(key, o);
        }
        else // No
        {
            m_Table[index].Set(key, o);
            return tIterator(&m_Table[index], this);
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element only if the key does not already exist by moving the value (C++11).
     */
    tIterator InsertUnique(const K &key, T &&o)
    {
        int index = XFindPos(key);

        if (m_Table[index].m_Status == STATUS_OCCUPIED)
        {
            return tIterator(&m_Table[index], this);
        }

        // If this insertion would exceed threshold, rehash first to avoid moving twice.
        if (m_Count + 1 >= m_Threshold)
        {
            Rehash(m_Table.Size() * 2);
            index = XFindPos(key);
        }

        if (m_Table[index].m_Status != STATUS_DELETED)
        {
            ++m_Occupation;
            ++m_Count;
        }
        else
        {
            ++m_Count;
        }

        m_Table[index].Set(key, std::move(o));
        return tIterator(&m_Table[index], this);
    }

    /**
     * @brief Inserts an element only if the key does not already exist and reports whether it was new (C++11).
     */
    tPair TestInsert(const K &key, const T &o)
    {
        int index = XFindPos(key);

        if (m_Table[index].m_Status == STATUS_OCCUPIED)
        {
            return tPair(tIterator(&m_Table[index], this), FALSE);
        }

        if (m_Count + 1 >= m_Threshold)
        {
            Rehash(m_Table.Size() * 2);
            index = XFindPos(key);
        }

        if (m_Table[index].m_Status != STATUS_DELETED)
        {
            ++m_Occupation;
            ++m_Count;
        }
        else
        {
            ++m_Count;
        }

        m_Table[index].Set(key, o);
        return tPair(tIterator(&m_Table[index], this), TRUE);
    }

    /**
     * @brief Inserts an element only if the key does not already exist and reports whether it was new by moving the value (C++11).
     */
    tPair TestInsert(const K &key, T &&o)
    {
        int index = XFindPos(key);

        if (m_Table[index].m_Status == STATUS_OCCUPIED)
        {
            return tPair(tIterator(&m_Table[index], this), FALSE);
        }

        if (m_Count + 1 >= m_Threshold)
        {
            Rehash(m_Table.Size() * 2);
            index = XFindPos(key);
        }

        if (m_Table[index].m_Status != STATUS_DELETED)
        {
            ++m_Occupation;
            ++m_Count;
        }
        else
        {
            ++m_Count;
        }

        m_Table[index].Set(key, std::move(o));
        return tPair(tIterator(&m_Table[index], this), TRUE);
    }

    /**
     * @brief Constructs a value in-place (via temporary) and inserts/overwrites (C++11).
     */
    template <class... Args>
    tIterator Emplace(const K &key, XBOOL override, Args &&...args)
    {
        Insert(key, T(std::forward<Args>(args)...), override);
        return tIterator(XFindIndex(key), this);
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
     * @remarks The element is not physically removed but marked as DELETED to preserve the probing chain.
     */
    void Remove(const K &key)
    {
        int index = XFindPos(key);
        if (m_Table[index].m_Status == STATUS_OCCUPIED)
        {
            m_Table[index].m_Status = STATUS_DELETED;
            --m_Count;
        }
    }

    /**
     * @brief Removes an element using an iterator.
     * @param it An iterator pointing to the element to remove.
     * @return An iterator to the element following the one that was removed.
     * @remarks The element is marked as DELETED.
     */
    tIterator Remove(const tIterator &it)
    {
        // may be not necessary
        pEntry e = it.m_Node;
        if (e == m_Table.End())
            return it;

        if (e->m_Status == STATUS_OCCUPIED)
        {
            e->m_Status = STATUS_DELETED;
            --m_Count;
        }

        ++e;
        while (e != m_Table.End())
        {
            // we're not at the end of the list yet
            if (e->m_Status == STATUS_OCCUPIED)
                break;
            ++e;
        }

        return tIterator(e, this);
    }

    /**
     * @brief Accesses an element by key.
     * @param key The key of the element to access.
     * @return A reference to the element's value.
     * @remarks If no element corresponds to the key, a new element is created and inserted into the table.
     */
    T &operator[](const K &key)
    {
        // Insert x as active
        int index = XFindPos(key);

        if (m_Table[index].m_Status != STATUS_OCCUPIED)
        {
            // If the element was deleted, we remove an element
            if ((m_Table[index].m_Status != STATUS_DELETED))
            {
                ++m_Occupation;
                ++m_Count;
            }
            else
            {
                ++m_Count;
            }
            m_Table[index].m_Status = STATUS_OCCUPIED;
            m_Table[index].m_Key = key;
        }

        // Test the rehash need
        if (m_Occupation < m_Threshold)
            return m_Table[index].m_Data;

        Rehash(m_Table.Size() * 2);
        return m_Table[XFindPos(key)].m_Data;
    }

    /**
     * @brief Finds an element by key.
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
        pEntry e = XFindIndex(key);
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
        pEntry e = XFindIndex(key);
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
    int IsHere(const K &key) const
    {
        return (int)XFindIndex(key);
    }

    /**
     * @brief Returns an iterator to the first element in the hash table.
     * @return An `Iterator` to the first element. If the table is empty, returns `End()`.
     *
     * @example
     * @code
     * XSHashTableIt<T,K,H> it = h.Begin();
     * XSHashTableIt<T,K,H> itend = h.End();
     *
     * for(; it != itend; ++it) {
     *     // do something with *t
     * }
     * @endcode
     */
    tIterator Begin()
    {
        for (pEntry it = m_Table.Begin(); it != m_Table.End(); it++)
        {
            if (it->m_Status == STATUS_OCCUPIED)
                return tIterator(it, this);
        }
        return End();
    }

    /**
     * @brief Returns a constant iterator to the first element in the hash table.
     * @return A `ConstIterator` to the first element. If the table is empty, returns `End()`.
     */
    tConstIterator Begin() const
    {
        for (pEntry it = m_Table.Begin(); it != m_Table.End(); it++)
        {
            if (it->m_Status == STATUS_OCCUPIED)
                return tConstIterator(it, this);
        }
        return End();
    }

    /**
     * @brief Returns an iterator pointing past the last element of the hash table.
     * @return An `Iterator` representing the end of the container.
     */
    tIterator End()
    {
        return tIterator(m_Table.End(), this);
    }

    /**
     * @brief Returns a constant iterator pointing past the last element of the hash table.
     * @return A `ConstIterator` representing the end of the container.
     */
    tConstIterator End() const
    {
        return tConstIterator(m_Table.End(), this);
    }

    /**
     * @brief Calculates the initial bucket index for a given key.
     * @param key The key to hash.
     * @return The initial index of the bucket in the hash table.
     */
    int Index(const K &key) const
    {
        H hashfun;
        return XIndex(hashfun(key), m_Table.Size());
    }

    /**
     * @brief Returns the number of elements in the hash table.
     * @return The total number of occupied elements.
     */
    int Size() const
    {
        return m_Count;
    }

private:
    /**
     * @brief Gets a pointer to the start of the bucket array.
     * @return A const pointer to the first bucket entry.
     */
    pEntry *GetFirstBucket() const { return m_Table.ConstBegin(); }

    /**
     * @brief Resizes the hash table and re-inserts all existing elements.
     * @param size The new number of buckets for the table.
     */
    void
    Rehash(int size)
    {
        int oldsize = m_Table.Size();
        m_Threshold = (int)(size * m_LoadFactor);

        // Temporary table
        XClassArray<tEntry> tmp;
        tmp.Resize(size);

        m_Table.Swap(tmp);
        m_Count = 0;
        m_Occupation = 0;

        for (int index = 0; index < oldsize; ++index)
        {
            pEntry first = &tmp[index];

            if (first->m_Status == STATUS_OCCUPIED)
            {
                Insert(first->m_Key, first->m_Data, TRUE);
            }
        }
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
     * @brief Internal helper to copy data from another hash table.
     * @param a The source hash table to copy from.
     */
    void XCopy(const XSHashTable &a)
    {
        m_Table = a.m_Table;
        m_Count = a.m_Count;
        m_Occupation = a.m_Occupation;
        m_Threshold = a.m_Threshold;
        m_LoadFactor = a.m_LoadFactor;
    }

#if VX_HAS_CXX11
    /**
     * @brief Internal helper to move data from another hash table.
     * @param a The source hash table to move from.
     */
    void XMove(XSHashTable &&a)
    {
        m_Table = std::move(a.m_Table);
        m_Count = a.m_Count;
        m_Occupation = a.m_Occupation;
        m_Threshold = a.m_Threshold;
        m_LoadFactor = a.m_LoadFactor;
        a.m_Count = 0;
        a.m_Occupation = 0;
        a.m_Threshold = 0;
        a.m_LoadFactor = 0.0f;
    }
#endif

    /**
     * @brief Finds an entry by key.
     * @param key The key to find.
     * @return A pointer to the found entry, or NULL if not found or the entry is not occupied.
     */
    pEntry XFindIndex(const K &key) const
    {
        int index = XFindPos(key);
        if (index < 0)
            return NULL;
        pEntry e = &m_Table[index];
        if (e->m_Status == STATUS_OCCUPIED)
            return e;
        else
            return NULL;
    }

    /**
     * @brief Finds the position for a key using linear probing.
     * @param key The key to find.
     * @return The index of the bucket where the key is or should be inserted. Returns -1 if the table is full and the key is not found.
     */
    int XFindPos(const K &key) const
    {
        int index = Index(key);
        int oldindex = index;

        Eq equalFunc;

        while (m_Table[index].m_Status == STATUS_OCCUPIED)
        {
            if (equalFunc(m_Table[index].m_Key, key))
                return index;
            ++index; // Compute ith probe
            if (index == m_Table.Size())
                index = 0;
            if (index == oldindex)
                return -1;
        }
        return index;
    }

    /// @brief The array of entries that holds the hash table data.
    XClassArray<tEntry> m_Table;
    /// @brief The number of elements currently in the hash table (occupied entries).
    int m_Count;
    /// @brief The number of occupied or deleted entries. Used to decide when to rehash.
    int m_Occupation;
    /// @brief The threshold at which the table will be rehashed (m_Table.Size() * m_LoadFactor).
    int m_Threshold;
    /// @brief The load factor for the hashtable.
    float m_LoadFactor;
};

#endif // XSHASHTABLE_H
