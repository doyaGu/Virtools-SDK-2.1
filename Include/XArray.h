#ifndef XARRAY_H
#define XARRAY_H

#include "VxMathDefines.h"
#include "XUtil.h"

#if VX_HAS_CXX11
#include <initializer_list>
#include <utility>
#endif

/**
 * @class XArray
 * @brief A template class for a dynamic array.
 *
 * @tparam T The type of elements to be stored in the array.
 *
 * @remarks
 * The array manages its own memory and automatically grows as needed.
 * The reserved capacity doubles each time the current capacity is reached.
 * The capacity may be reduced if the number of elements falls below 30% of the reserved size.
 * This class should not be used to store objects of variable size or objects requiring
 * explicit construction/destruction when the array is resized. Use XClassArray for such cases.
 *
 * @see XClassArray, XSArray
 */
template <class T>
class XArray
{
public:
    // Types
    typedef T *Iterator;             ///< A pointer to an element, used as an iterator.
    typedef const T *ConstIterator;  ///< A pointer to a const element, used as a const iterator.
    typedef T Type;                  ///< The type of the elements stored in the array.
    typedef T &Reference;            ///< A reference to an element.
    typedef const T &ConstReference; ///< A const reference to an element.

    /**
     * @brief Constructs an empty array, optionally reserving space.
     * @param ss The initial number of elements to reserve space for.
     */
    explicit XArray(int ss = 0)
    {
        if (ss > 0)
        {
            m_Begin = Allocate(ss);
            m_End = m_Begin;
            m_AllocatedEnd = m_Begin + ss;
        }
        else
        {
            m_AllocatedEnd = NULL;
            m_Begin = m_End = NULL;
        }
    }

    /**
     * @brief Copy constructor. Creates a deep copy of another array.
     * @param a The array to copy from.
     */
    XArray(const XArray<T> &a)
    {
        int size = a.Size();
        if (size > 0)
        {
            m_Begin = Allocate(size);
            m_End = m_Begin + size;
            m_AllocatedEnd = m_End;
            XCopy(m_Begin, a.m_Begin, a.m_End);
        }
        else
        {
            m_AllocatedEnd = NULL;
            m_Begin = m_End = NULL;
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Constructs the array from an initializer list (C++11).
     * @param init The initializer list.
     */
    XArray(std::initializer_list<T> init)
    {
        int size = (int)init.size();
        if (size > 0)
        {
            m_Begin = Allocate(size);
            m_End = m_Begin;
            m_AllocatedEnd = m_Begin + size;
            for (const auto &v : init)
            {
                *(m_End++) = v;
            }
        }
        else
        {
            m_AllocatedEnd = NULL;
            m_Begin = m_End = NULL;
        }
    }
#endif

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @param a The array to move from.
     */
    XArray(XArray<T> &&a) VX_NOEXCEPT
    {
        m_Begin = a.m_Begin;
        m_End = a.m_End;
        m_AllocatedEnd = a.m_AllocatedEnd;
        a.m_Begin = NULL;
        a.m_End = NULL;
        a.m_AllocatedEnd = NULL;
    }
#endif

    /**
     * @brief Destructor.
     * @remarks Releases the memory allocated by the array. If the array stores pointers,
     * it is the user's responsibility to delete the pointed-to objects before destroying the array.
     */
    ~XArray() { Free(); }

    /**
     * @brief Assignment operator. Replaces the array's content with a copy of another array's content.
     * @param a The array to assign from.
     * @return A reference to this array.
     */
    XArray<T> &operator=(const XArray<T> &a)
    {
        if (this != &a)
        {
            int size = a.Size();
            if (size == 0)
            {
                m_End = m_Begin;
                if (!m_Begin)
                {
                    m_End = NULL;
                }
            }
            else if (Allocated() >= size)
            {
                XCopy(m_Begin, a.m_Begin, a.m_End);
                m_End = m_Begin + size;
            }
            else
            {
                Free();
                m_Begin = Allocate(size);
                m_End = m_Begin + size;
                m_AllocatedEnd = m_End;
                XCopy(m_Begin, a.m_Begin, a.m_End);
            }
        }
        return *this;
    }

#if VX_HAS_CXX11
    /**
     * @brief Assignment from initializer list (C++11).
     * @param init The initializer list.
     * @return A reference to this array.
     */
    XArray<T> &operator=(std::initializer_list<T> init)
    {
        int size = (int)init.size();
        if (size == 0)
        {
            m_End = m_Begin;
            if (!m_Begin)
            {
                m_End = NULL;
            }
            return *this;
        }

        if (Allocated() < size)
        {
            Free();
            m_Begin = Allocate(size);
            m_AllocatedEnd = m_Begin + size;
        }

        T *out = m_Begin;
        for (const auto &v : init)
        {
            *(out++) = v;
        }
        m_End = out;
        return *this;
    }
#endif

#if VX_HAS_CXX11
    /**
     * @brief Move assignment operator (C++11).
     * @param a The array to move from.
     * @return A reference to this array.
     */
    XArray<T> &operator=(XArray<T> &&a) VX_NOEXCEPT
    {
        if (this != &a)
        {
            Free();
            m_Begin = a.m_Begin;
            m_End = a.m_End;
            m_AllocatedEnd = a.m_AllocatedEnd;
            a.m_Begin = NULL;
            a.m_End = NULL;
            a.m_AllocatedEnd = NULL;
        }
        return *this;
    }
#endif

    /**
     * @brief Appends the contents of another array to the end of this one.
     * @param a The array whose elements are to be appended.
     * @return A reference to this array.
     */
    XArray<T> &operator+=(const XArray<T> &a)
    {
        int size = a.Size();
        if (size)
        {
            int oldsize = Size();
            int newSize = oldsize + size + 1;
            if (newSize <= Allocated())
            {
                // we recopy the new array
                XCopy(m_End, a.m_Begin, a.m_End);
                m_End += size;
            }
            else
            {
                T *temp = Allocate(newSize);

                // we recopy the old array
                XCopy(temp, m_Begin, m_End);
                m_End = temp + oldsize;

                // we copy the given array
                XCopy(m_End, a.m_Begin, a.m_End);
                m_End += size;
                m_AllocatedEnd = m_End + 1;

                // we free the old memory
                Free();

                // we set the new pointer
                m_Begin = temp;
            }
        }
        return *this;
    }

    /**
     * @brief Removes all elements from this array that are also present in another array.
     * @param a The array containing elements to remove.
     * @return A reference to this array.
     */
    XArray<T> &operator-=(const XArray<T> &a)
    {
        int size = a.Size();
        if (size)
        {
            int oldsize = Size();
            if (oldsize)
            {
                T *newarray = Allocate(oldsize + 1);
                T *temp = newarray;

                for (T *t = m_Begin; t != m_End; ++t)
                {
                    // we search for the element in the other array
                    if (a.Find(*t) == a.m_End)
                    {
                        // the element is not in the other array, we copy it to the newone
                        *temp = *t;
                        ++temp;
                    }
                }

                Free();
                // we set the new pointers
                m_Begin = newarray;
                m_End = temp;
                m_AllocatedEnd = m_Begin + oldsize + 1;
            }
        }
        return *this;
    }

    /**
     * @brief Removes all elements from the array and frees all allocated memory.
     */
    void Clear()
    {
        Free();
        m_Begin = NULL;
        m_End = NULL;
        m_AllocatedEnd = NULL;
    }

    /**
     * @brief Reduces the allocated capacity to match the current size of the array.
     */
    void Compact()
    {
        if (!m_Begin)
            return;
        if (m_AllocatedEnd > m_End)
        {
            int size = Size();
            if (size == 0)
                return;
            T *newData = Allocate(size);
            XCopy(newData, m_Begin, m_End);
            Free();
            m_Begin = newData;
            m_AllocatedEnd = m_End = newData + size;
        }
    }

    /**
     * @brief Reserves memory for a specified number of elements.
     * @remarks If the new size is smaller than the current size, elements are discarded.
     * @param size The number of elements to reserve space for.
     */
    void Reserve(int size)
    {
        T *newData = Allocate(size);

        // Recopy of old elements
        int oldCount = Size();
        if (oldCount > 0)
        {
            T *last = XMin(m_Begin + size, m_End);
            XCopy(newData, m_Begin, last);
            oldCount = (int)(last - m_Begin);
        }
        else
        {
            oldCount = 0;
        }

        // new Pointers
        Free();
        m_Begin = newData;
        m_End = newData ? (newData + oldCount) : NULL;
        m_AllocatedEnd = newData ? (newData + size) : NULL;
    }

    /**
     * @brief Resizes the array to contain a specified number of elements.
     * @param size The new size of the array.
     * @remarks After `Resize(n)`, elements from index 0 to n-1 can be accessed.
     * No constructors are called for new elements. If the new size is larger than the
     * current capacity, the array is reallocated. `Resize(0)` is faster than `Clear()`
     * if you intend to add more elements later.
     * @see Reserve
     */
    void Resize(int size)
    {
        XASSERT(size >= 0);
        if (size == 0 && !m_Begin)
        {
            m_End = NULL;
            return;
        }
        if (size > Allocated())
        {
            Reserve(size);
        }
        m_End = m_Begin + size;
    }

    /**
     * @brief Increases the size of the array by a given number of elements.
     * @param e The number of elements to add to the size.
     * @remarks Memory is reallocated if necessary. The new elements are uninitialized.
     */
    void Expand(int e = 1)
    {
        int newSize = Size() + e;
        if (newSize > Allocated())
        {
            int newCapacity = Allocated() ? Allocated() * 2 : 2;
            while (newSize > newCapacity)
            {
                newCapacity *= 2;
            }
            Reserve(newCapacity);
        }
        m_End += e;
    }

    /**
     * @brief Decreases the size of the array by a given number of elements from the end.
     * @param e The number of elements to remove from the size.
     */
    void Compress(int e = 1)
    {
        if (m_Begin + e <= m_End)
        {
            m_End -= e;
        }
        else
        {
            m_End = m_Begin;
        }
    }

    /**
     * @brief Inserts an element at the end of the array.
     * @param o The element to insert.
     */
    void PushBack(const T &o)
    {
        if (m_End == m_AllocatedEnd)
        {
            Reserve(Size() ? Size() * 2 : 2);
        }
        *(m_End++) = o;
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element at the end of the array by moving it (C++11).
     * @param o The element to insert.
     */
    void PushBack(T &&o)
    {
        if (m_End == m_AllocatedEnd)
        {
            Reserve(Size() ? Size() * 2 : 2);
        }
        *(m_End++) = std::move(o);
    }

    /**
     * @brief Constructs an element at the end of the array (C++11).
     * @remarks Implemented via assignment into an existing slot.
     */
    template <class... Args>
    void EmplaceBack(Args &&...args)
    {
        if (m_End == m_AllocatedEnd)
        {
            Reserve(Size() ? Size() * 2 : 2);
        }
        *(m_End++) = T(std::forward<Args>(args)...);
    }
#endif

    /**
     * @brief Inserts an element at the beginning of the array.
     * @param o The element to insert.
     * @remarks This operation can be slow for large arrays as it requires shifting all existing elements.
     */
    void PushFront(const T &o)
    {
        XInsert(m_Begin, o);
    }

    /**
     * @brief Inserts an element at a specified position.
     * @param i An iterator (pointer) to the element before which to insert.
     * @param o The element to insert.
     */
    void Insert(T *i, const T &o)
    {
        if (i >= m_Begin && i <= m_End)
            XInsert(i, o);
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element at a specified position by moving it (C++11).
     * @param i An iterator (pointer) to the element before which to insert.
     * @param o The element to insert.
     */
    void Insert(T *i, T &&o)
    {
        if (i >= m_Begin && i <= m_End)
            XInsert(i, std::move(o));
    }

    /**
     * @brief Constructs and inserts an element at a specified position (C++11).
     * @remarks Implemented via assignment into an existing slot.
     */
    template <class... Args>
    void Emplace(T *i, Args &&...args)
    {
        if (i >= m_Begin && i <= m_End)
            XInsert(i, T(std::forward<Args>(args)...));
    }
#endif

    /**
     * @brief Inserts an element at a specified index.
     * @param pos The index at which to insert the element.
     * @param o The element to insert.
     */
    void Insert(int pos, const T &o)
    {
        Insert(m_Begin + pos, o);
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element at a specified index by moving it (C++11).
     */
    void Insert(int pos, T &&o)
    {
        Insert(m_Begin + pos, std::move(o));
    }
#endif

    /**
     * @brief Inserts an element into a sorted array while maintaining order.
     * @param o The element to insert.
     * @remarks The array must already be sorted according to the `<` operator of type T.
     */
    void InsertSorted(const T &o)
    {
        T *begin = m_Begin;
        T *end = m_End;
        while (begin < end)
        {
            T *pivot = begin + ((end - begin) >> 1);
            if (o < *pivot)
                end = pivot;
            else
                begin = pivot + 1;
        }
        XInsert(begin, o);
    }

    /**
     * @brief Moves an element from one position to another.
     * @param i An iterator to the position before which the element will be inserted.
     * @param n An iterator to the element to be moved.
     */
    void Move(T *i, T *n)
    {
        if (i >= m_Begin && i <= m_End && n >= m_Begin && n < m_End)
        {
            int insertpos = i - m_Begin;
            if (n < i)
                --insertpos;
            T tn = *n;
#if VX_HAS_CXX11
            tn = std::move(*n);
#endif
            XRemove(n);
            Insert(insertpos, tn);
        }
    }

    /**
     * @brief Removes and returns the last element of the array.
     * @return The element that was removed.
     * @remarks The behavior is undefined if the array is empty.
     */
    T PopBack()
    {
        XASSERT(!IsEmpty());
        T t = *(m_End - 1);
        XRemove(m_End - 1);
        return t;
    }

    /**
     * @brief Removes the first element of the array.
     */
    void PopFront()
    {
        if (!IsEmpty())
        {
            XRemove(m_Begin);
        }
    }

    /**
     * @brief Removes an element at a given index and returns its value.
     * @param pos The index of the element to remove.
     * @param old A reference to store the value of the removed element.
     * @return TRUE if an element was removed, FALSE if the index was out of bounds.
     */
    XBOOL RemoveAt(unsigned int pos, T &old)
    {
        T *t = m_Begin + pos;
        if (t >= m_End)
            return FALSE;
        old = *t;
        XRemove(t);
        return TRUE;
    }

    /**
     * @brief Erases an element at a given index.
     * @param pos The index of the element to erase.
     * @return TRUE if the element was erased, FALSE otherwise.
     */
    XBOOL EraseAt(int pos)
    {
        return (XBOOL)Remove(m_Begin + pos);
    }

    /**
     * @brief Removes an element at a given index.
     * @param pos The index of the element to remove.
     * @return An iterator to the element that followed the removed element.
     */
    T *RemoveAt(int pos)
    {
        return Remove(m_Begin + pos);
    }

    /**
     * @brief Removes the element at the specified position.
     * @param i An iterator pointing to the element to remove.
     * @return An iterator to the element that followed the removed element, or NULL if the iterator was invalid.
     */
    T *Remove(T *i)
    {
        if (i >= m_Begin && i < m_End)
        {
            return XRemove(i);
        }
        return NULL;
    }

    /**
     * @brief Removes the first occurrence of a specific element.
     * @param o The element to remove.
     * @return An iterator to the element that followed the removed element, or NULL if the element was not found.
     */
    T *Remove(const T &o)
    {
        T *t = Find(o);
        if (t < m_End)
        {
            return XRemove(t);
        }
        return NULL;
    }

    /**
     * @brief Erases the first occurrence of a specific element.
     * @param o The element to erase.
     * @return TRUE if the element was found and erased, FALSE otherwise.
     */
    XBOOL Erase(const T &o)
    {
        T *t = Find(o);
        if (t < m_End)
        {
            XRemove(t);
            return TRUE;
        }
        return FALSE;
    }

    /**
     * @brief Quickly removes an element by swapping it with the last element. Does not preserve order.
     * @param o The element to remove.
     */
    void FastRemove(const T &o)
    {
        FastRemove(Find(o));
    }

    /**
     * @brief Quickly removes an element by swapping it with the last element. Does not preserve order.
     * @param iT An iterator to the element to remove.
     */
    void FastRemove(const Iterator &iT)
    {
        if (iT >= m_Begin && iT < m_End)
        {
            --m_End;
            if (iT < m_End)
            {
                *iT = *m_End;
            }
        }
    }

    /**
     * @brief Quickly removes an element at a given index. Does not preserve order.
     * @param pos The index of the element to remove.
     */
    void FastRemoveAt(int pos)
    {
        FastRemove(Begin() + pos);
    }

    /**
     * @brief Fills the entire array with a specified value.
     * @param o The value to fill the array with.
     */
    void Fill(const T &o)
    {
        for (T *t = m_Begin; t != m_End; ++t)
            *t = o;
    }

    /**
     * @brief Fills the array's memory with a specific byte value.
     * @param val The byte value to set.
     */
    void Memset(XBYTE val)
    {
        if (m_Begin)
            memset(m_Begin, val, Size() * sizeof(T));
    }

    /**
     * @brief Provides const access to an element by its index.
     * @param i The index of the element.
     * @return A const reference to the element.
     * @remarks Asserts that the index is in bounds in debug builds.
     */
    const T &operator[](int i) const
    {
        XASSERT(i >= 0 && i < Size());
        return *(m_Begin + i);
    }

    /**
     * @brief Provides access to an element by its index.
     * @param i The index of the element.
     * @return A reference to the element.
     * @remarks Asserts that the index is in bounds in debug builds.
     */
    T &operator[](int i)
    {
        XASSERT(i >= 0 && i < Size());
        return *(m_Begin + i);
    }

    /**
     * @brief Provides safe access to an element by its index.
     * @param i The index of the element.
     * @return A pointer to the element, or End() if the index is out of bounds.
     */
    T *At(unsigned int i)
    {
        T *t = m_Begin + i;
        return (t < m_End) ? t : m_End;
    }

    /**
     * @brief Provides safe const access to an element by its index.
     * @param i The index of the element.
     * @return A const pointer to the element, or End() if the index is out of bounds.
     */
    const T *At(unsigned int i) const
    {
        const T *t = m_Begin + i;
        return (t < m_End) ? t : m_End;
    }

    /**
     * @brief Finds the first occurrence of an element in the array.
     * @param o The element to find.
     * @return An iterator to the first found element, or End() if not found.
     */
    T *Find(const T &o) const
    {
        T *t = m_Begin;
        while (t < m_End && *t != o)
            ++t;
        return t;
    }

    /**
     * @brief Finds an element in a sorted array using binary search.
     * @param o The element to find.
     * @return An iterator to the found element, or NULL if not found.
     * @remarks The array must be sorted in increasing order for this to work correctly.
     */
    T *BinaryFind(const T &o) const
    {
        int low = 0;
        int high = Size() - 1;
        while (low <= high)
        {
            int mid = (low + high) >> 1;
            if (m_Begin[mid] > o)
            {
                high = mid - 1;
            }
            else if (m_Begin[mid] < o)
            {
                low = mid + 1;
            }
            else
            {
                return m_Begin + mid;
            }
        }
        return NULL;
    }

    /**
     * @brief Checks if an element is present in the array.
     * @param o The element to check for.
     * @return TRUE if the element is found, FALSE otherwise.
     */
    XBOOL IsHere(const T &o) const
    {
        return Find(o) != m_End;
    }

    /**
     * @brief Gets the index of the first occurrence of an element.
     * @param o The element to find.
     * @return The zero-based index of the element, or -1 if not found.
     */
    int GetPosition(const T &o) const
    {
        T *t = Find(o);
        return (t == m_End) ? -1 : (t - m_Begin);
    }

    /**
     * @brief Swaps two elements in the array.
     * @param pos1 The index of the first element.
     * @param pos2 The index of the second element.
     */
    void Swap(int pos1, int pos2)
    {
        char buffer[sizeof(T)];
        memcpy(buffer, m_Begin + pos1, sizeof(T));
        memcpy(m_Begin + pos1, m_Begin + pos2, sizeof(T));
        memcpy(m_Begin + pos2, buffer, sizeof(T));
    }

    /**
     * @brief Swaps the contents of this array with another.
     * @param a The other array to swap with.
     */
    void Swap(XArray<T> &a)
    {
        XSwap(m_Begin, a.m_Begin);
        XSwap(m_End, a.m_End);
        XSwap(m_AllocatedEnd, a.m_AllocatedEnd);
    }

    /**
     * @brief Attaches the array to a pre-existing C-style array without taking ownership.
     * @param iArray Pointer to the external array.
     * @param iCount The number of elements in the external array.
     */
    void Attach(T *iArray, int iCount)
    {
        Clear();
        m_Begin = iArray;
        m_End = m_Begin + iCount;
        m_AllocatedEnd = m_End; // No extra allocation
    }

    /**
     * @brief Detaches the array from any external data it was attached to.
     */
    void Detach()
    {
        m_Begin = NULL;
        m_End = NULL;
        m_AllocatedEnd = NULL;
    }

    /**
     * @brief Returns a reference to the first element.
     * @remarks Behavior is undefined if the array is empty.
     */
    T &Front() { return *Begin(); }
    const T &Front() const { return *Begin(); }

    /**
     * @brief Returns a reference to the last element.
     * @remarks Behavior is undefined if the array is empty.
     */
    T &Back() { return *(End() - 1); }
    const T &Back() const { return *(End() - 1); }

    /**
     * @brief Returns an iterator to the beginning of the array.
     * @example
     * @code
     * for(XArray<int>::Iterator it = arr.Begin(); it != arr.End(); ++it) {
     *     // do something with *it
     * }
     * @endcode
     */
    T *Begin() const { return m_Begin; }

    /**
     * @brief STL-compatible begin() for range-for and algorithms.
     */
    T *begin() { return m_Begin; }
    const T *begin() const { return m_Begin; }
    const T *cbegin() const { return m_Begin; }

    /**
     * @brief Returns a reverse iterator to the end of the array.
     */
    T *RBegin() const { return m_End - 1; }

    /**
     * @brief Returns an iterator to the position after the last element.
     */
    T *End() const { return m_End; }

    /**
     * @brief STL-compatible end() for range-for and algorithms.
     */
    T *end() { return m_End; }
    const T *end() const { return m_End; }
    const T *cend() const { return m_End; }

    /**
     * @brief Returns a reverse iterator to the position before the first element.
     */
    T *REnd() const { return m_Begin - 1; }

    /**
     * @brief Returns the number of elements in the array.
     */
    int Size() const { return m_Begin ? (int)(m_End - m_Begin) : 0; }

    /**
     * @brief Checks if the array is empty.
     * @return True if the array is empty, false otherwise.
     * @remarks Faster than checking `Size() == 0`.
     */
    bool IsEmpty() const { return m_End == m_Begin; }

    /**
     * @brief Returns the total memory occupied by the allocated buffer in bytes.
     * @param addstatic If TRUE, adds the `sizeof(XArray)` to the result.
     * @return The memory size in bytes.
     */
    int GetMemoryOccupation(XBOOL addstatic = FALSE) const
    {
        return Allocated() * sizeof(T) + (addstatic ? sizeof(*this) : 0);
    }

    /**
     * @brief Returns the number of elements the array can hold without reallocating.
     */
    int Allocated() const { return m_Begin ? (int)(m_AllocatedEnd - m_Begin) : 0; }

    /**
     * @brief A default comparison function for sorting.
     * @internal
     */
    static int XCompare(const void *elem1, const void *elem2)
    {
        return (*(T *)elem1 > *(T *)elem2) ? 1 : ((*(T *)elem1 < *(T *)elem2) ? -1 : 0);
    }

    /**
     * @brief Sorts the array using the C standard library's `qsort`.
     * @param compare An optional pointer to a comparison function.
     */
    void Sort(VxSortFunc compare = XCompare)
    {
        if (Size() > 1)
            qsort(m_Begin, Size(), sizeof(T), compare);
    }

    /**
     * @brief Sorts a specified range of the array using bubble sort.
     * @param rangestart An iterator to the start of the range to sort.
     * @param rangeend An iterator to the end of the range to sort.
     * @param compare An optional pointer to a comparison function.
     */
    void BubbleSort(T *rangestart, T *rangeend, VxSortFunc compare = XCompare)
    {
        if (!compare || (rangeend - rangestart) <= 1)
            return;
        XBOOL Noswap = TRUE;
        for (T *it1 = rangestart + 1; it1 < rangeend; it1++)
        {
            for (T *it2 = rangeend - 1; it2 >= it1; it2--)
            {
                if (compare(it2, it2 - 1) < 0)
                {
                    XSwap(*it2, *(it2 - 1));
                    Noswap = FALSE;
                }
            }
            if (Noswap)
                break;
            Noswap = TRUE;
        }
    }

    /**
     * @brief Sorts the entire array using bubble sort.
     * @param compare An optional pointer to a comparison function.
     */
    void BubbleSort(VxSortFunc compare = XCompare)
    {
        BubbleSort(m_Begin, m_End, compare);
    }

protected:
    /** @name Internal Memory Management */
    ///@{

    /**
     * @brief Copies a block of memory.
     * @internal
     */
    void XCopy(T *dest, T *start, T *end)
    {
        if (start == end)
            return;
        int size = ((XBYTE *)end - (XBYTE *)start);
        if (size > 0)
            memcpy(dest, start, size);
    }

    /**
     * @brief Moves a block of memory, handling overlapping regions correctly.
     * @internal
     */
    void XMove(T *dest, T *start, T *end)
    {
        if (start == end)
            return;
        int size = ((XBYTE *)end - (XBYTE *)start);
        if (size > 0)
            memmove(dest, start, size);
    }

    /**
     * @brief Inserts an element, handling reallocation if necessary.
     * @internal
     */
    void XInsert(T *i, const T &o)
    {
        XASSERT(i >= m_Begin);
        XASSERT(i <= m_End);

        // Test For Reallocation
        if (m_End == m_AllocatedEnd)
        {
            int newSize = Allocated() * 2;
            if (newSize == 0)
                newSize = 2;
            T *newData = Allocate(newSize);

            // copy before insertion point
            XCopy(newData, m_Begin, i);

            // copy the new element
            T *insertionPoint = newData + (i - m_Begin);
            *(insertionPoint) = o;

            // copy after insertion point
            XCopy(insertionPoint + 1, i, m_End);

            // New Pointers
            m_End = newData + (m_End - m_Begin);
            Free();
            m_Begin = newData;
            m_AllocatedEnd = newData + newSize;
        }
        else
        {
            // copy after insertion point
            XMove(i + 1, i, m_End);
            // copy the new element
            *i = o;
        }
        ++m_End;
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element by moving it (C++11).
     * @internal
     */
    void XInsert(T *i, T &&o)
    {
        XASSERT(i >= m_Begin);
        XASSERT(i <= m_End);

        // Test For Reallocation
        if (m_End == m_AllocatedEnd)
        {
            int newSize = Allocated() * 2;
            if (newSize == 0)
                newSize = 2;
            T *newData = Allocate(newSize);

            // copy before insertion point
            XCopy(newData, m_Begin, i);

            // copy the new element
            T *insertionPoint = newData + (i - m_Begin);
            *(insertionPoint) = std::move(o);

            // copy after insertion point
            XCopy(insertionPoint + 1, i, m_End);

            // New Pointers
            m_End = newData + (m_End - m_Begin);
            Free();
            m_Begin = newData;
            m_AllocatedEnd = newData + newSize;
        }
        else
        {
            // copy after insertion point
            XMove(i + 1, i, m_End);
            // copy the new element
            *i = std::move(o);
        }
        ++m_End;
    }
#endif

    /**
     * @brief Removes an element by shifting subsequent elements.
     * @internal
     */
    T *XRemove(T *i)
    {
        XMove(i, i + 1, m_End);
        --m_End;
        return i;
    }

    /**
     * @brief Allocates raw memory for a given number of elements.
     * @internal
     */
    T *Allocate(int size)
    {
        if (size > 0)
#ifndef VX_MALLOC
            return new T[size];
#else
            return (T *)VxMalloc(sizeof(T) * size);
#endif
        else
            return NULL;
    }

    /**
     * @brief Frees the raw memory block.
     * @internal
     */
    void Free()
    {
        if (m_Begin)
#ifndef VX_MALLOC
            delete[] m_Begin;
#else
            VxFree(m_Begin);
#endif
    }

    ///@}

    /// @name Members
    ///@{
    T *m_Begin;        ///< @internal Pointer to the beginning of the allocated memory.
    T *m_End;          ///< @internal Pointer to the position after the last element.
    T *m_AllocatedEnd; ///< @internal Pointer to the end of the allocated memory block.
    ///@}
};

/// @brief A convenient typedef for an array of void pointers.
typedef XArray<void *> XVoidArray;

#endif // XARRAY_H
