#ifndef XHASHTABLE_H
#define XHASHTABLE_H

#include "XClassArray.h"
#include "XArray.h"
#include "XHashFun.h"

#if VX_HAS_CXX11
#include <initializer_list>
#include <utility>
#endif

#ifdef VX_MSVC
#pragma warning(disable : 4786)
#endif

/// @brief The default load factor that determines when the hash table should be resized.
#define LOAD_FACTOR 0.75f

template <class T, class K, class H, class Eq>
class XHashTable;

/**
 * @class XHashTableEntry
 * @brief Represents a single entry (key-value pair) within a hash table bucket.
 *
 * @tparam T The type of the data stored.
 * @tparam K The type of the key used for hashing and lookup.
 *
 * @remarks This class is used internally by XHashTable to manage collisions via chaining.
 * Each entry points to the next entry in the same bucket, forming a linked list.
 */
template <class T, class K>
class XHashTableEntry
{
    typedef XHashTableEntry<T, K> *tEntry;

public:
    /**
     * @brief Default constructor.
     */
    XHashTableEntry() : m_Key(), m_Data(), m_Next(NULL) {}

    /**
     * @brief Constructor to initialize with a key and a value.
     * @param k The key for the entry.
     * @param v The data for the entry.
     */
    XHashTableEntry(const K &k, const T &v) : m_Key(k), m_Data(v), m_Next(NULL) {}

    /**
     * @brief Copy constructor.
     * @param e The entry to copy.
     */
    XHashTableEntry(const XHashTableEntry<T, K> &e) : m_Key(e.m_Key), m_Data(e.m_Data), m_Next(e.m_Next) {}

    /**
     * @brief Copy assignment operator.
     * @param e The entry to copy from.
     * @return Reference to this entry.
     */
    XHashTableEntry<T, K> &operator=(const XHashTableEntry<T, K> &e)
    {
        if (this != &e)
        {
            m_Key = e.m_Key;
            m_Data = e.m_Data;
            m_Next = e.m_Next;
        }
        return *this;
    }

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @param e The entry to move from.
     */
    XHashTableEntry(XHashTableEntry<T, K> &&e) VX_NOEXCEPT : m_Key(std::move(e.m_Key)), m_Data(std::move(e.m_Data)), m_Next(e.m_Next)
    {
        e.m_Next = NULL;
    }

    /**
     * @brief Move assignment operator (C++11).
     * @param e The entry to move from.
     * @return Reference to this entry.
     */
    XHashTableEntry<T, K> &operator=(XHashTableEntry<T, K> &&e) VX_NOEXCEPT
    {
        if (this != &e)
        {
            m_Key = std::move(e.m_Key);
            m_Data = std::move(e.m_Data);
            m_Next = e.m_Next;
            e.m_Next = NULL;
        }
        return *this;
    }
#endif
    /**
     * @brief Destructor.
     */
    ~XHashTableEntry() {}

    /// The key associated with this entry.
    K m_Key;
    /// The data stored in this entry.
    T m_Data;
    /// Pointer to the next entry in the same bucket (for collision resolution).
    tEntry m_Next;
};

/**
 * @class XHashTableIt
 * @brief An iterator for traversing an XHashTable.
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
 * XHashTableIt<MyType, MyKey, MyHash> it = hashtable.Begin();
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
class XHashTableIt
{
    typedef XHashTableEntry<T, K> *tEntry;
    typedef XHashTableIt<T, K, H, Eq> tIterator;
    typedef XHashTable<T, K, H, Eq> *tTable;

    friend class XHashTable<T, K, H, Eq>;

public:
    /**
     * @brief Default constructor. Initializes a null iterator.
     */
    XHashTableIt() : m_Node(0), m_Table(0) {}

    /**
     * @brief Copy constructor.
     * @param n The iterator to copy.
     */
    XHashTableIt(const tIterator &n) : m_Node(n.m_Node), m_Table(n.m_Table) {}

    /**
     * @brief Copy assignment operator.
     * @param n The iterator to copy from.
     * @return Reference to this iterator.
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
    XHashTableIt(tIterator &&n) VX_NOEXCEPT : m_Node(n.m_Node), m_Table(n.m_Table)
    {
        n.m_Node = NULL;
        n.m_Table = NULL;
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
     * @brief Gets the key of the entry pointed to by the iterator.
     * @return A reference to the key.
     */
    K &GetKey() { return m_Node->m_Key; }

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
     * @brief Internal constructor used by XHashTable.
     * @param n Pointer to the hash table entry.
     * @param t Pointer to the hash table container.
     */
    XHashTableIt(tEntry n, tTable t) : m_Node(n), m_Table(t) {}

    /// Pointer to the current hash table entry.
    tEntry m_Node;

    /// Pointer to the hash table this iterator belongs to.
    tTable m_Table;
};

/**
 * @class XHashTableConstIt
 * @brief A constant iterator for traversing an XHashTable.
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
 *     XHashTableConstIt<MyType, MyKey, MyHash> it = m_Hashtable.Begin();
 *     while (it != m_Hashtable.End()) {
 *         // access the key
 *         it.GetKey();
 *
 *         // access the element (read-only)
 *         const MyType& value = *it;
 *
 *         // move to the next element
 *         ++it;
 *     }
 * }
 * @endcode
 */
template <class T, class K, class H = XHashFun<K>, class Eq = XEqual<K>>
class XHashTableConstIt
{
    typedef XHashTableEntry<T, K> *tEntry;
    typedef XHashTableConstIt<T, K, H, Eq> tConstIterator;
    typedef XHashTable<T, K, H, Eq> const *tConstTable;
    friend class XHashTable<T, K, H, Eq>;

public:
    /**
     * @brief Default constructor. Initializes a null iterator.
     */
    XHashTableConstIt() : m_Node(0), m_Table(0) {}

    /**
     * @brief Copy constructor.
     * @param n The iterator to copy.
     */
    XHashTableConstIt(const tConstIterator &n) : m_Node(n.m_Node), m_Table(n.m_Table) {}

    /**
     * @brief Copy assignment operator.
     * @param n The iterator to copy from.
     * @return Reference to this iterator.
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
    XHashTableConstIt(tConstIterator &&n) VX_NOEXCEPT : m_Node(n.m_Node), m_Table(n.m_Table)
    {
        n.m_Node = NULL;
        n.m_Table = NULL;
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
     * @brief Pre-increment operator. Advances the iterator to the next element.
     * @return A reference to the incremented iterator.
     */
    tConstIterator &operator++()
    {
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
     * @brief Internal constructor used by XHashTable.
     * @param n Pointer to the hash table entry.
     * @param t Pointer to the const hash table container.
     */
    XHashTableConstIt(tEntry n, tConstTable t) : m_Node(n), m_Table(t) {}

    /// Pointer to the current hash table entry.
    tEntry m_Node;

    /// Pointer to the const hash table this iterator belongs to.
    tConstTable m_Table;
};

/**
 * @class XHashTablePair
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
class XHashTablePair
{
public:
    /**
     * @brief Constructor for the pair.
     * @param it Iterator to the element.
     * @param n A boolean value; TRUE if the element was newly inserted, FALSE if it already existed.
     */
    XHashTablePair(XHashTableIt<T, K, H, Eq> it, int n) : m_Iterator(it), m_New(n) {}

    /// An iterator pointing to the inserted or found element.
    XHashTableIt<T, K, H, Eq> m_Iterator;
    /// A flag indicating if the element was newly inserted (TRUE) or already present (FALSE).
    XBOOL m_New;
};

/**
 * @class XHashTable
 * @brief A container class implementing a hash table.
 *
 * @tparam T The type of the element to insert.
 * @tparam K The type of the key.
 * @tparam H The hash function object used to hash the key. See XHashFun.h for default implementations.
 * @tparam Eq The equality comparison object for keys.
 *
 * @remarks
 * This implementation of the hash table uses a linked list in each bucket to resolve
 * hash collisions. This means memory is allocated for each element upon insertion.
 * For a static implementation without dynamic allocation per element, see XSHashTable.
 *
 * The table automatically resizes and rehashes its elements when the number of elements
 * exceeds the product of the number of buckets and the load factor `L`.
 */
template <class T, class K, class H = XHashFun<K>, class Eq = XEqual<K> /*, float L = 0.75f*/>
class XHashTable
{
    // Types
    typedef XHashTable<T, K, H, Eq> tTable;
    typedef XHashTableEntry<T, K> *tEntry;
    typedef XHashTableIt<T, K, H, Eq> tIterator;
    typedef XHashTableConstIt<T, K, H, Eq> tConstIterator;
    typedef XHashTablePair<T, K, H, Eq> tPair;

    /// @brief Friend class declaration for the iterator.
    friend class XHashTableIt<T, K, H, Eq>;
    /// @brief Friend class declaration for the const iterator.
    friend class XHashTableConstIt<T, K, H, Eq>;

public:
    /// @typedef Entry The type of a single entry in the hash table.
    typedef XHashTableEntry<T, K> Entry;
    /// @typedef Pair The type returned by TestInsert, containing an iterator and a boolean.
    typedef XHashTablePair<T, K, H, Eq> Pair;
    /// @typedef Iterator The mutable iterator for this hash table.
    typedef XHashTableIt<T, K, H, Eq> Iterator;
    /// @typedef ConstIterator The constant iterator for this hash table.
    typedef XHashTableConstIt<T, K, H, Eq> ConstIterator;

    /**
     * @brief Default constructor.
     * @param initialize The initial number of buckets. Should be a power of 2; otherwise, it will be rounded up to the next power of 2.
     */
    explicit XHashTable(int initialize = 16)
    {
        initialize = Near2Power(initialize);
        if (initialize < 4)
            initialize = 4;

        m_Table.Resize(initialize);
        m_Table.Fill(0);
        m_Pool.Reserve((int)(initialize * LOAD_FACTOR));
    }

    /**
     * @brief Copy constructor.
     * @param a The hash table to copy.
     */
    XHashTable(const XHashTable &a) { XCopy(a); }

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @param a The hash table to move from.
     */
    XHashTable(XHashTable &&a) VX_NOEXCEPT { XMove(std::move(a)); }

    /**
     * @brief Constructs the table from an initializer list of key/value pairs (C++11).
     */
    XHashTable(std::initializer_list<std::pair<K, T>> init, int initialize = 16)
        : XHashTable(((int)init.size() * 2 > initialize) ? (int)init.size() * 2 : initialize)
    {
        for (const auto &kv : init)
        {
            Insert(kv.first, kv.second);
        }
    }
#endif

    /**
     * @brief Destructor.
     * @remarks Releases the elements contained in the hash table. If you are storing pointers,
     * you must iterate over the table and manually delete each pointed-to object before destroying the table.
     */
    ~XHashTable() {}

    /**
     * @brief Removes all elements from the table.
     * @remarks The number of buckets remains unchanged. The memory pool for entries is cleared.
     */
    void Clear()
    {
        // we clear all the allocated entries
        m_Pool.Resize(0);
        // we clear the table
        m_Table.Fill(0);
    }

    /**
     * @brief Analyzes the distribution of elements across buckets.
     * @param[out] iBucketOccupation An array that will be filled with the occupation statistics.
     * The index of the array represents the number of elements in a bucket, and the value is the count of buckets with that many elements.
     * `iBucketOccupation[0]` will store the count of empty buckets.
     */
    void GetOccupation(XArray<int> &iBucketOccupation) const
    {
        iBucketOccupation.Resize(1);
        iBucketOccupation[0] = 0;

        for (tEntry *it = m_Table.Begin(); it != m_Table.End(); it++)
        {
            if (!*it) // there is someone there
            {
                iBucketOccupation[0]++;
            }
            else
            {
                // count the number of occupant
                int count = 1;
                tEntry e = *it;
                while (e->m_Next)
                {
                    e = e->m_Next;
                    count++;
                }

                int oldsize = iBucketOccupation.Size();
                if (oldsize <= count) // we need to resize
                {
                    iBucketOccupation.Resize(count + 1);

                    // and we init to 0
                    for (int i = oldsize; i <= count; ++i)
                        iBucketOccupation[i] = 0;
                }

                // the recensing
                iBucketOccupation[count]++;
            }
        }
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
            if (m_Pool.Size() == m_Pool.Allocated()) // Need Rehash
            {
                Rehash(m_Table.Size() * 2);
                return Insert(key, o, override);
            }
            else // No
            {
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
        while (true)
        {
            int index = Index(key);
            tEntry e = XFind(index, key);
            if (!e)
            {
                if (m_Pool.Size() == m_Pool.Allocated())
                {
                    Rehash(m_Table.Size() * 2);
                    continue;
                }
                XInsert(index, key, std::move(o));
                return TRUE;
            }

            if (!override)
                return FALSE;

            e->m_Data = std::move(o);
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
    Iterator Insert(const K &key, const T &o)
    {
        int index = Index(key);
        Eq equalFunc;

        // we look for existing key
        for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
        {
            if (equalFunc(e->m_Key, key))
            {
                e->m_Data = o;
                return Iterator(e, this);
            }
        }

        if (m_Pool.Size() == m_Pool.Allocated())
        {
            // Need Rehash
            Rehash(m_Table.Size() * 2);
            return Insert(key, o);
        }
        else
        {
            // No
            return Iterator(XInsert(index, key, o), this);
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts or updates an element by moving the value (C++11).
     */
    Iterator Insert(const K &key, T &&o)
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
                    return Iterator(e, this);
                }
            }

            if (m_Pool.Size() == m_Pool.Allocated())
            {
                Rehash(m_Table.Size() * 2);
                continue;
            }

            return Iterator(XInsert(index, key, std::move(o)), this);
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
    Pair TestInsert(const K &key, const T &o)
    {
        int index = Index(key);
        Eq equalFunc;

        // we look for existing key
        for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
        {
            if (equalFunc(e->m_Key, key))
            {
                return Pair(Iterator(e, this), 0);
            }
        }

        // Need Rehash
        if (m_Pool.Size() == m_Pool.Allocated())
        {
            // Need Rehash
            Rehash(m_Table.Size() * 2);
            return TestInsert(key, o);
        }
        else
        {
            // No
            return Pair(Iterator(XInsert(index, key, o), this), 1);
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element and reports whether it was new by moving the value (C++11).
     */
    Pair TestInsert(const K &key, T &&o)
    {
        Eq equalFunc;

        while (true)
        {
            int index = Index(key);
            for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
            {
                if (equalFunc(e->m_Key, key))
                {
                    return Pair(Iterator(e, this), 0);
                }
            }

            if (m_Pool.Size() == m_Pool.Allocated())
            {
                Rehash(m_Table.Size() * 2);
                continue;
            }

            return Pair(Iterator(XInsert(index, key, std::move(o)), this), 1);
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
    Iterator InsertUnique(const K &key, const T &o)
    {
        int index = Index(key);
        Eq equalFunc;

        // we look for existing key
        for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
        {
            if (equalFunc(e->m_Key, key))
            {
                return Iterator(e, this);
            }
        }

        if (m_Pool.Size() == m_Pool.Allocated()) // Need Rehash
        {
            Rehash(m_Table.Size() * 2);
            return InsertUnique(key, o);
        }
        else // No
        {
            return Iterator(XInsert(index, key, o), this);
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element only if the key does not already exist by moving the value (C++11).
     */
    Iterator InsertUnique(const K &key, T &&o)
    {
        Eq equalFunc;

        while (true)
        {
            int index = Index(key);
            for (tEntry e = m_Table[index]; e != 0; e = e->m_Next)
            {
                if (equalFunc(e->m_Key, key))
                {
                    return Iterator(e, this);
                }
            }

            if (m_Pool.Size() == m_Pool.Allocated())
            {
                Rehash(m_Table.Size() * 2);
                continue;
            }

            return Iterator(XInsert(index, key, std::move(o)), this);
        }
    }

    /**
     * @brief Constructs a value in-place (via temporary) and inserts/overwrites (C++11).
     */
    template <class... Args>
    Iterator Emplace(const K &key, Args &&...args)
    {
        return Insert(key, T(std::forward<Args>(args)...));
    }

    /**
     * @brief Constructs a value in-place (via temporary) and inserts if missing (C++11).
     */
    template <class... Args>
    Iterator EmplaceUnique(const K &key, Args &&...args)
    {
        return InsertUnique(key, T(std::forward<Args>(args)...));
    }

    /**
     * @brief Constructs a value in-place (via temporary) and inserts if missing, reporting whether it was new (C++11).
     */
    template <class... Args>
    Pair TestEmplace(const K &key, Args &&...args)
    {
        return TestInsert(key, T(std::forward<Args>(args)...));
    }

    /// @brief STL-compatible begin/end for range-for and algorithms (C++11).
    Iterator begin() { return Begin(); }
    ConstIterator begin() const { return Begin(); }
    ConstIterator cbegin() const { return Begin(); }

    Iterator end() { return End(); }
    ConstIterator end() const { return End(); }
    ConstIterator cend() const { return End(); }
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

                // change the pointers to it
                if (old)
                {
                    old->m_Next = e->m_Next;
                }
                else
                {
                    m_Table[index] = e->m_Next;
                }

                // then remove it from the pool
                // NOTE: FastRemove compacts the pool by moving the last entry into the removed slot.
                // If the moved entry's m_Next pointed to the removed slot, the move would create a
                // self-referential link (cycle). Fix that up before the move.
                tEntry oldLast = (m_Pool.Size() > 0) ? (m_Pool.End() - 1) : nullptr;
                if (oldLast && oldLast != e && oldLast->m_Next == e)
                {
                    oldLast->m_Next = e->m_Next;
                }

                m_Pool.FastRemove(e);
                if (e != m_Pool.End()) // wasn't the last one... we need to remap
                {
                    RematEntry(oldLast, e);
                }

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
    Iterator Remove(const tIterator &it)
    {
        int index = Index(it.m_Node->m_Key);
        if (index >= m_Table.Size())
            return Iterator(0, this);

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
                    old = old->m_Next;
                }
                else
                {
                    m_Table[index] = e->m_Next;
                    old = m_Table[index];
                }

                // then remove it from the pool
                // See Remove(key) for why we fix up oldLast->m_Next.
                tEntry oldLast = (m_Pool.Size() > 0) ? (m_Pool.End() - 1) : nullptr;
                if (oldLast && oldLast != e && oldLast->m_Next == e)
                {
                    oldLast->m_Next = e->m_Next;
                }

                m_Pool.FastRemove(e);
                if (e != m_Pool.End()) // wasn't the last one... we need to remap
                {
                    RematEntry(oldLast, e);
                    if (old == m_Pool.End())
                        old = e;
                }

                break;
            }
            old = e;
        }
        // There is an element in the same column, we return it
        if (!old) // No element in the same bucket, we parse for the next
        {
            while (!old && ++index < m_Table.Size())
                old = m_Table[index];
        }

        return Iterator(old, this);
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
            if (m_Pool.Size() == m_Pool.Allocated()) // Need Rehash
            {
                Rehash(m_Table.Size() * 2);
                return operator[](key);
            }
            else // No
            {
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
    Iterator Find(const K &key)
    {
        return Iterator(XFindIndex(key), this);
    }

    /**
     * @brief Finds an element by key in a constant hash table.
     * @param key The key of the element to find.
     * @return A `ConstIterator` to the found element, or `End()` if the element is not found.
     */
    ConstIterator Find(const K &key) const
    {
        return ConstIterator(XFindIndex(key), this);
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
        return XFindIndex(key) != NULL;
    }

    /**
     * @brief Returns an iterator to the first element in the hash table.
     * @return An `Iterator` to the first element. If the table is empty, returns `End()`.
     *
     * @example
     * @code
     * XHashTable<T,K,H>::Iterator it = h.Begin();
     * XHashTable<T,K,H>::Iterator itend = h.End();
     *
     * for(; it != itend; ++it) {
     *     // do something with *it
     * }
     * @endcode
     */
    Iterator Begin()
    {
        for (tEntry *it = m_Table.Begin(); it != m_Table.End(); it++)
        {
            if (*it)
                return Iterator(*it, this);
        }
        return End();
    }

    /**
     * @brief Returns a constant iterator to the first element in the hash table.
     * @return A `ConstIterator` to the first element. If the table is empty, returns `End()`.
     */
    ConstIterator Begin() const
    {
        for (tEntry *it = m_Table.Begin(); it != m_Table.End(); it++)
        {
            if (*it)
                return ConstIterator(*it, this);
        }
        return End();
    }

    /**
     * @brief Returns an iterator pointing past the last element of the hash table.
     * @return An `Iterator` representing the end of the container.
     */
    Iterator End()
    {
        return Iterator(0, this);
    }

    /**
     * @brief Returns a constant iterator pointing past the last element of the hash table.
     * @return A `ConstIterator` representing the end of the container.
     */
    ConstIterator End() const
    {
        return ConstIterator(0, this);
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
        return m_Pool.Size();
    }

    /**
     * @brief Calculates the memory usage of the hash table.
     * @param addstatic If TRUE, includes the size of the `XHashTable` object itself in the calculation.
     * @return The total memory occupied in bytes.
     */
    int GetMemoryOccupation(XBOOL addstatic = FALSE) const
    {
        return m_Table.GetMemoryOccupation(addstatic) +
               m_Pool.Allocated() * sizeof(Entry) +
               (addstatic ? sizeof(*this) : 0);
    }

    /**
     * @brief Reserves memory for an expected number of elements to avoid rehashes.
     * @param iCount The number of elements to reserve space for.
     * @remarks This function should be called before populating the hash table for best performance.
     * It adjusts the size of the internal bucket array and reserves space in the element pool.
     */
    void Reserve(const int iCount)
    {
        int requiredCount = iCount;
        if (requiredCount < 0)
            requiredCount = 0;
        if (requiredCount < Size())
            requiredCount = Size();

        // Compute a bucket count that can hold requiredCount at the target load factor.
        // Never shrink the table; Reserve is intended to reduce rehashing.
        int tableSize = Near2Power((int)((float)requiredCount / LOAD_FACTOR) + 1);
        if (tableSize < 4)
            tableSize = 4;
        if (tableSize < m_Table.Size())
            tableSize = m_Table.Size();

        if (tableSize != m_Table.Size() || requiredCount > m_Pool.Allocated())
        {
            Rehash(tableSize);
        }
    }

private:
    /**
     * @brief Gets a pointer to the start of the bucket array.
     * @return A pointer to the first bucket.
     */
    tEntry *GetFirstBucket() const { return m_Table.Begin(); }

    /**
     * @brief Resizes and rehashes the entire table.
     * @param iSize The new number of buckets for the table.
     */
    void Rehash(int iSize)
    {
        int oldsize = m_Table.Size();

        // we create a new pool
        XClassArray<Entry> pool((int)(iSize * LOAD_FACTOR));
        pool = m_Pool;

        // Temporary table
        XArray<tEntry> tmp;
        tmp.Resize(iSize);
        tmp.Fill(0);

        for (int index = 0; index < oldsize; ++index)
        {
            tEntry first = m_Table[index];
            while (first)
            {
                H hashfun;
                int newindex = XIndex(hashfun(first->m_Key), iSize);

                Entry *newe = pool.Begin() + (first - m_Pool.Begin());

                // insert new entry in new table
                newe->m_Next = tmp[newindex];
                tmp[newindex] = newe;

                first = first->m_Next;
            }
        }
        m_Table.Swap(tmp);
        m_Pool.Swap(pool);
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
    void XCopy(const XHashTable &a)
    {
        int size = a.m_Table.Size();
        m_Table.Resize(size);
        m_Table.Fill(0);

        m_Pool.Reserve(a.m_Pool.Allocated());
        m_Pool = a.m_Pool;

        // remap the address in the table
        for (int i = 0; i < size; ++i)
        {
            if (a.m_Table[i])
                m_Table[i] = m_Pool.Begin() + (a.m_Table[i] - a.m_Pool.Begin());
        }

        // remap the addresses in the entries
        for (Entry *e = m_Pool.Begin(); e != m_Pool.End(); ++e)
        {
            if (e->m_Next)
            {
                e->m_Next = m_Pool.Begin() + (e->m_Next - a.m_Pool.Begin());
            }
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Internal helper to move data from another hash table.
     * @param a The source hash table to move from.
     */
    void XMove(XHashTable &&a)
    {
        m_Table = std::move(a.m_Table);
        m_Pool = std::move(a.m_Pool);
    }
#endif

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
        tEntry newe = GetFreeEntry();
        newe->m_Key = key;
        newe->m_Data = o;
        newe->m_Next = m_Table[index];
        m_Table[index] = newe;
        return newe;
    }

#if VX_HAS_CXX11
    tEntry XInsert(int index, const K &key, T &&o)
    {
        tEntry newe = GetFreeEntry();
        newe->m_Key = key;
        newe->m_Data = std::move(o);
        newe->m_Next = m_Table[index];
        m_Table[index] = newe;
        return newe;
    }
#endif

    /**
     * @brief Gets a new, uninitialized entry from the memory pool.
     * @return A pointer to the new entry.
     */
    tEntry GetFreeEntry()
    {
        // We consider when we arrive here that we have space
        m_Pool.Resize(m_Pool.Size() + 1);
        return (m_Pool.End() - 1);
    }

    /**
     * @brief Remaps pointers after a FastRemove operation on the pool.
     * @param iOld The old address of the moved entry.
     * @param iNew The new address of the moved entry.
     */
    void RematEntry(tEntry iOld, tEntry iNew)
    {
        int index = Index(iNew->m_Key);
        XASSERT(m_Table[index]);

        if (m_Table[index] == iOld) // It was the first of the bucket
        {
            m_Table[index] = iNew;
        }
        else
        {
            for (tEntry n = m_Table[index]; n->m_Next != NULL; n = n->m_Next)
            {
                if (n->m_Next == iOld) // found one
                {
                    n->m_Next = iNew;
                    break; // only one can match
                }
            }
        }
    }

    /// @brief The array of buckets, where each bucket is a pointer to the first entry in a linked list.
    XArray<tEntry> m_Table;
    /// @brief The memory pool that stores all hash table entries contiguously.
    XClassArray<Entry> m_Pool;
};

#endif // XHASHTABLE_H
