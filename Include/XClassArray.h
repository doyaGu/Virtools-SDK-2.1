#ifndef XCLASSARRAY_H
#define XCLASSARRAY_H

#include "VxMathDefines.h"
#include "XUtil.h"

#if VX_HAS_CXX11
#include <initializer_list>
#include <utility>
#endif

#ifdef VX_MSVC
#pragma warning(disable : 4786)
#endif

/**
 * @class XClassArray
 * @brief A dynamic array designed to hold classes or structures.
 *
 * @tparam T The type of the class or structure to be stored.
 *
 * @remarks
 * This array is specifically designed for elements that require proper
 * construction, destruction, or copy assignment handling (e.g., classes
 * with pointers or other managed resources). Unlike XArray, it uses
 * copy assignment and does not rely on `memcpy`, ensuring correct behavior
 * for complex types.
 *
 * @see XArray, XSArray
 */
template <class T>
class XClassArray
{
public:
    typedef T *Iterator;            ///< A pointer to an element, used as an iterator.
    typedef const T *ConstIterator; ///< A pointer to a const element, used as a const iterator.

    /**
     * @brief Constructs an empty array, optionally reserving space.
     * @param ss The initial number of elements to reserve space for.
     */
    explicit XClassArray(int ss = 0)
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
     * @brief Copy constructor. Creates a deep copy of another array using element-wise copy assignment.
     * @param a The array to copy from.
     */
    XClassArray(const XClassArray<T> &a)
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
    XClassArray(std::initializer_list<T> init)
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
    XClassArray(XClassArray<T> &&a) VX_NOEXCEPT
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
     * @remarks Releases the memory for the array. Note that if `T` is a pointer type,
     * the objects pointed to are *not* deleted. You must iterate and delete them manually
     * before the array is destroyed.
     */
    ~XClassArray()
    {
        Clear();
    }

    /**
     * @brief Assignment operator. Replaces the array's content with a copy of another array's content.
     * @param a The array to assign from.
     * @return A reference to this array.
     */
    XClassArray<T> &operator=(const XClassArray<T> &a)
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
    XClassArray<T> &operator=(std::initializer_list<T> init)
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
    XClassArray<T> &operator=(XClassArray<T> &&a) VX_NOEXCEPT
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
     * @brief Reserves memory for a specified number of elements.
     * @remarks If the new size is smaller than the current size, elements are discarded.
     * @param size The number of elements to reserve space for.
     */
    void Reserve(int size)
    {
        // allocation of new size
        T *newdata = Allocate(size);

        // Recopy of old elements
        int oldCount = Size();
        if (oldCount > 0)
        {
            T *last = XMin(m_Begin + size, m_End);
            XCopy(newdata, m_Begin, last);
            oldCount = (int)(last - m_Begin);
        }
        else
        {
            oldCount = 0;
        }

        // new Pointers
        Free();
        m_Begin = newdata;
        m_End = newdata ? (newdata + oldCount) : NULL;
        m_AllocatedEnd = newdata ? (newdata + size) : NULL;
    }

    /**
     * @brief Resizes the array to contain a specified number of elements.
     * @param size The new size of the array.
     * @remarks If the new size is larger than the current size, new default-constructed
     * elements are added. If smaller, elements are removed from the end.
     */
    void Resize(int size)
    {
        if (size > Allocated())
        {
            Reserve(size);
        }
        m_End = m_Begin + size;
    }

    /**
     * @brief Inserts an element at the end of the array.
     * @param o The element to insert.
     */
    void PushBack(const T &o)
    {
        XInsert(m_End, o);
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element at the end of the array by moving it (C++11).
     */
    void PushBack(T &&o) { XInsert(m_End, std::move(o)); }

    /**
     * @brief Constructs an element at the end of the array (C++11).
     * @remarks Implemented via assignment into an existing slot.
     */
    template <class... Args>
    void EmplaceBack(Args &&...args)
    {
        XInsert(m_End, T(std::forward<Args>(args)...));
    }
#endif

    /**
     * @brief Increases the size of the array by a given number of default-constructed elements.
     * @param e The number of elements to add to the size.
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
     * @brief Inserts an element at the beginning of the array.
     * @param o The element to insert.
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
        {
            XInsert(i, o);
        }
    }

#if VX_HAS_CXX11
    void Insert(T *i, T &&o)
    {
        if (i >= m_Begin && i <= m_End)
        {
            XInsert(i, std::move(o));
        }
    }

    template <class... Args>
    void Emplace(T *i, Args &&...args)
    {
        if (i >= m_Begin && i <= m_End)
        {
            XInsert(i, T(std::forward<Args>(args)...));
        }
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
    void Insert(int pos, T &&o) { Insert(m_Begin + pos, std::move(o)); }
#endif

    /**
     * @brief Removes the last element of the array.
     */
    void PopBack()
    {
        if (m_End > m_Begin)
            XRemove(m_End - 1);
    }

    /**
     * @brief Removes the first element of the array.
     */
    void PopFront()
    {
        if (m_Begin != m_End)
            XRemove(m_Begin);
    }

    /**
     * @brief Removes an element at a specified position.
     * @param i An iterator pointing to the element to remove.
     * @return An iterator to the element that followed the removed element, or NULL if the iterator was invalid.
     */
    T *Remove(T *i)
    {
        if (i >= m_Begin && i < m_End)
            return XRemove(i);
        return NULL;
    }

    /**
     * @brief Removes an element at a specified index.
     * @param pos The index of the element to remove.
     * @return An iterator to the element that followed the removed element, or NULL if the index was out of bounds.
     */
    T *RemoveAt(int pos)
    {
        if (pos >= Size())
            return NULL;
        return XRemove(m_Begin + pos);
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
            m_End--;
            if (iT < m_End)
            {
                *iT = *m_End;
            }
        }
    }

    /**
     * @brief Provides const access to an element by its index.
     * @param i The index of the element.
     * @return A const reference to the element.
     */
    T &operator[](int i) const
    {
        XASSERT(i >= 0 && i < Size());
        return *(m_Begin + i);
    }

    /**
     * @brief Provides safe const access to an element by its index.
     * @param i The index of the element.
     * @return A pointer to the element, or End() if the index is out of bounds.
     */
    T *At(unsigned int i) const
    {
        if (i >= (unsigned int)Size())
            return m_End;
        return m_Begin + i;
    }

    /**
     * @brief Swaps two elements in the array.
     * @param pos1 The index of the first element.
     * @param pos2 The index of the second element.
     */
    void Swap(int pos1, int pos2)
    {
        T temp = *(m_Begin + pos1);
        *(m_Begin + pos1) = *(m_Begin + pos2);
        *(m_Begin + pos2) = temp;
    }

    /**
     * @brief Swaps the contents of this array with another.
     * @param a The other array to swap with.
     */
    void Swap(XClassArray<T> &a)
    {
        XSwap(m_Begin, a.m_Begin);
        XSwap(m_End, a.m_End);
        XSwap(m_AllocatedEnd, a.m_AllocatedEnd);
    }

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
     * for(XClassArray<MyClass>::Iterator it = arr.Begin(); it != arr.End(); ++it) {
     *     // do something with *it
     * }
     * @endcode
     */
    T *Begin() const { return m_Begin; }

    /// @brief STL-compatible begin() for range-for and algorithms.
    T *begin() { return m_Begin; }
    const T *begin() const { return m_Begin; }
    const T *cbegin() const { return m_Begin; }

    /**
     * @brief Returns an iterator to the position after the last element.
     */
    T *End() const { return m_End; }

    /// @brief STL-compatible end() for range-for and algorithms.
    T *end() { return m_End; }
    const T *end() const { return m_End; }
    const T *cend() const { return m_End; }

    /**
     * @brief Returns the number of elements in the array.
     */
    int Size() const { return m_Begin ? (int)(m_End - m_Begin) : 0; }

    /**
     * @brief Returns the number of elements the array can hold without reallocating.
     */
    int Allocated() const { return m_Begin ? (int)(m_AllocatedEnd - m_Begin) : 0; }

    /**
     * @brief Returns the total memory occupied by the allocated buffer in bytes.
     * @param addstatic If TRUE, adds the `sizeof(XClassArray)` to the result.
     * @return The memory size in bytes.
     */
    int GetMemoryOccupation(XBOOL addstatic = FALSE) const
    {
        return Allocated() * sizeof(T) + (addstatic ? sizeof(*this) : 0);
    }

    /**
     * @brief Finds the first occurrence of an element in the array.
     * @param o The element to find.
     * @return An iterator to the first found element, or End() if not found.
     */
    T *Find(const T &o) const
    {
        // If the array is empty
        if (!(m_End - m_Begin))
            return m_End;
        T *t = m_Begin;
        while (t < m_End && *t != o)
            ++t;
        return t;
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
     * @brief A default comparison function for sorting.
     * @internal
     */
    static int XCompare(const void *elem1, const void *elem2)
    {
        return (*(T *)elem1 > *(T *)elem2) ? 1 : ((*(T *)elem1 < *(T *)elem2) ? -1 : 0);
    }

    /**
     * @brief Sorts the array using the C standard library's `qsort`.
     * @param Fct An optional pointer to a comparison function.
     */
    void Sort(VxSortFunc Fct = XCompare)
    {
        if (Size() > 1)
        {
            qsort(m_Begin, Size(), sizeof(T), Fct);
        }
    }

protected:
    /// @name Internal Memory Management
    ///@{

    /**
     * @brief Copies elements using the assignment operator.
     * @internal
     */
    void XCopy(T *dest, T *start, T *end)
    {
        while (start != end)
        {
            *dest = *start;
            start++;
            dest++;
        }
    }

    /**
     * @brief Moves elements, handling overlapping regions correctly.
     * @internal
     */
    void XMove(T *dest, T *start, T *end)
    {
        if (dest > start)
        {
            dest += (end - start);
            while (start != end)
            {
                --dest;
                --end;
                *dest = *end;
            }
        }
        else
        {
            XCopy(dest, start, end);
        }
    }

    /**
     * @brief Inserts an element, handling reallocation if necessary.
     * @internal
     */
    void XInsert(T *i, const T &o)
    {
        // Test For Reallocation
        if (m_End == m_AllocatedEnd)
        {
            int newsize = (m_AllocatedEnd - m_Begin) * 2; //+m_AllocationSize;
            if (!newsize)
                newsize = 1;
            T *newdata = Allocate(newsize);

            // copy before insertion point
            XCopy(newdata, m_Begin, i);

            // copy the new element
            T *insertionpoint = newdata + (i - m_Begin);
            *(insertionpoint) = o;

            // copy after insertion point
            XCopy(insertionpoint + 1, i, m_End);

            // New Pointers
            m_End = newdata + (m_End - m_Begin);
            Free();
            m_Begin = newdata;
            m_AllocatedEnd = newdata + newsize;
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
    void XInsert(T *i, T &&o)
    {
        // Test For Reallocation
        if (m_End == m_AllocatedEnd)
        {
            int newsize = (m_AllocatedEnd - m_Begin) * 2;
            if (!newsize)
                newsize = 1;
            T *newdata = Allocate(newsize);

            // copy before insertion point
            XCopy(newdata, m_Begin, i);

            // copy the new element
            T *insertionpoint = newdata + (i - m_Begin);
            *(insertionpoint) = std::move(o);

            // copy after insertion point
            XCopy(insertionpoint + 1, i, m_End);

            // New Pointers
            m_End = newdata + (m_End - m_Begin);
            Free();
            m_Begin = newdata;
            m_AllocatedEnd = newdata + newsize;
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
     * @brief Allocates memory and default-constructs elements.
     * @internal
     */
    T *Allocate(int size)
    {
        if (size > 0)
        {
#ifndef VX_MALLOC
            return new T[size];
#else
            return VxAllocate<T>(size);
#endif
        }
        else
        {
            return NULL;
        }
    }

    /**
     * @brief Destructs elements and frees memory.
     * @internal
     */
    void Free()
    {
        if (m_Begin)
        {
#ifndef VX_MALLOC
            delete[] m_Begin;
#else
            VxDeallocate<T>(m_Begin, Allocated());
#endif
        }
    }

    ///@}

    /// @name Members
    ///@{
    T *m_Begin;        ///< @internal Pointer to the beginning of the allocated memory.
    T *m_End;          ///< @internal Pointer to the position after the last element.
    T *m_AllocatedEnd; ///< @internal Pointer to the end of the allocated memory block.
    ///@}
};

#endif // XCLASSARRAY_H
