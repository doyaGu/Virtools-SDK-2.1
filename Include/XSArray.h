#ifndef XSARRAY_H
#define XSARRAY_H

#include "VxMathDefines.h"
#include "XUtil.h"

#if VX_HAS_CXX11
#include <initializer_list>
#include <utility>
#endif

/**
 * @class XSArray
 * @brief A space-efficient dynamic array.
 *
 * @tparam T The type of elements to be stored in the array.
 *
 * @remarks
 * This array behaves similarly to `XArray`, but it does not reserve extra
 * capacity. Its allocated size is always exactly equal to the number of elements
 * it contains. This makes it more memory-efficient but can lead to frequent
 * reallocations (and thus be slower) when adding or removing elements.
 *
 * @see XClassArray, XArray
 */
template <class T>
class XSArray
{
public:
    /// @brief Default constructor. Creates an empty array.
    XSArray() : m_Begin(NULL), m_End(NULL) {}

    /**
     * @brief Copy constructor. Creates a deep copy of another array.
     * @param a The array to copy from.
     */
    XSArray(const XSArray<T> &a)
    {
        int size = a.Size();
        if (size > 0)
        {
            m_Begin = Allocate(size);
            m_End = m_Begin + size;
            XCopy(m_Begin, a.m_Begin, a.m_End);
        }
        else
        {
            m_Begin = NULL;
            m_End = NULL;
        }
    }

#if VX_HAS_CXX11
    /**
     * @brief Constructs the array from an initializer list (C++11).
     * @param init The initializer list.
     */
    XSArray(std::initializer_list<T> init)
    {
        int size = (int)init.size();
        if (size > 0)
        {
            m_Begin = Allocate(size);
            m_End = m_Begin;
            for (const auto &v : init)
            {
                *(m_End++) = v;
            }
        }
        else
        {
            m_Begin = NULL;
            m_End = NULL;
        }
    }
#endif

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @param a The array to move from.
     */
    XSArray(XSArray<T> &&a) VX_NOEXCEPT
    {
        m_Begin = a.m_Begin;
        m_End = a.m_End;
        a.m_Begin = NULL;
        a.m_End = NULL;
    }
#endif

    /**
     * @brief Destructor. Frees the allocated memory.
     */
    ~XSArray() { Clear(); }

    /**
     * @brief Assignment operator. Replaces the array's content with a copy of another array.
     * @param a The array to assign from.
     * @return A reference to this array.
     */
    XSArray<T> &operator=(const XSArray<T> &a)
    {
        if (this != &a)
        {
            Free();
            int size = a.Size();
            if (size > 0)
            {
                m_Begin = Allocate(size);
                m_End = m_Begin + size;
                XCopy(m_Begin, a.m_Begin, a.m_End);
            }
            else
            {
                m_Begin = NULL;
                m_End = NULL;
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
    XSArray<T> &operator=(std::initializer_list<T> init)
    {
        Free();
        int size = (int)init.size();
        if (size > 0)
        {
            m_Begin = Allocate(size);
            m_End = m_Begin;
            for (const auto &v : init)
            {
                *(m_End++) = v;
            }
        }
        else
        {
            m_Begin = NULL;
            m_End = NULL;
        }
        return *this;
    }
#endif

#if VX_HAS_CXX11
    /**
     * @brief Move assignment operator (C++11).
     * @param a The array to move from.
     * @return A reference to this array.
     */
    XSArray<T> &operator=(XSArray<T> &&a) VX_NOEXCEPT
    {
        if (this != &a)
        {
            Free();
            m_Begin = a.m_Begin;
            m_End = a.m_End;
            a.m_Begin = NULL;
            a.m_End = NULL;
        }
        return *this;
    }
#endif

    /**
     * @brief Appends the contents of another array to the end of this one.
     * @param a The array whose elements are to be appended.
     * @return A reference to this array.
     */
    XSArray<T> &operator+=(const XSArray<T> &a)
    {
        int size = a.Size();
        if (size > 0)
        {
            int oldsize = Size();
            T *temp = Allocate(oldsize + size);

            // we recopy the old array
            XCopy(temp, m_Begin, m_End);
            m_End = temp + oldsize;

            // we copy the given array
            XCopy(m_End, a.m_Begin, a.m_End);
            m_End += size;

            // we free the old memory
            Free();

            // we set the new pointer
            m_Begin = temp;
        }
        return *this;
    }

    /**
     * @brief Removes all elements from this array that are also present in another array.
     * @param a The array containing elements to remove.
     * @return A reference to this array.
     */
    XSArray<T> &operator-=(const XSArray<T> &a)
    {
        if (!a.Size() || !Size())
            return *this;
        T *newarray = Allocate(Size());
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

        // we free the memory
        Free();
        // the resize
        int size = temp - newarray;
        m_Begin = Allocate(size);
        m_End = m_Begin + size;
        // The copy
        XCopy(m_Begin, newarray, temp);
        return *this;
    }

    /**
     * @brief Removes all elements and frees the allocated memory.
     */
    void Clear()
    {
        Free();
        m_Begin = NULL;
        m_End = NULL;
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
     * @brief Resizes the array, reallocating memory.
     * @param size The new size of the array.
     */
    void Resize(int size)
    {
        if (size == Size())
            return;
        T *newdata = Allocate(size);

        // Recopy of old elements
        int oldCount = Size();
        int toCopy = XMin(oldCount, size);
        if (toCopy > 0)
        {
            XCopy(newdata, m_Begin, m_Begin + toCopy);
        }

        // new Pointers
        Free();
        m_Begin = newdata;
        m_End = newdata + size;
    }

    /// @brief Adds an element to the end of the array.
    void PushBack(const T &o) { XInsert(m_End, o); }

#if VX_HAS_CXX11
    /// @brief Adds an element to the end of the array by moving it (C++11).
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

    /// @brief Adds an element to the beginning of the array.
    void PushFront(const T &o) { XInsert(m_Begin, o); }

    /// @brief Inserts an element at a specified position.
    void Insert(T *i, const T &o)
    {
        if (i >= m_Begin && i <= m_End)
            XInsert(i, o);
    }

#if VX_HAS_CXX11
    /// @brief Inserts an element at a specified position by moving it (C++11).
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

    /// @brief Inserts an element at a specified index.
    void Insert(int pos, const T &o) { Insert(m_Begin + pos, o); }

#if VX_HAS_CXX11
    /// @brief Inserts an element at a specified index by moving it (C++11).
    void Insert(int pos, T &&o) { Insert(m_Begin + pos, std::move(o)); }
#endif

    /// @brief Moves an element from one position to another.
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

    /// @brief Removes the last element of the array.
    void PopBack()
    {
        if (m_End > m_Begin)
            XRemove(m_End - 1);
    }

    /// @brief Removes the first element of the array.
    void PopFront()
    {
        if (m_Begin != m_End)
            XRemove(m_Begin);
    }

    /// @brief Removes an element at a specified position.
    T *Remove(T *i)
    {
        if (i >= m_Begin && i < m_End)
            return XRemove(i);
        return NULL;
    }

    /// @brief Removes an element at a given index and returns its value.
    XBOOL RemoveAt(unsigned int pos, T &old)
    {
        T *t = m_Begin + pos;
        if (t >= m_End)
            return FALSE;
        old = *t;
        XRemove(t);
        return TRUE;
    }

    /// @brief Removes an element at a given index.
    T *RemoveAt(int pos)
    {
        if (pos >= 0 && m_Begin + pos < m_End)
            return XRemove(m_Begin + pos);
        return NULL;
    }

    /// @brief Removes the first occurrence of a specific element.
    int Remove(const T &o)
    {
        T *t = Find(o);
        if (t != m_End)
        {
            XRemove(t);
            return 1;
        }
        return 0;
    }

    /// @brief Provides access to an element by its index.
    T &operator[](unsigned int i) const { return *(m_Begin + i); }

    /// @brief Provides safe access to an element by its index.
    T *At(unsigned int i) const
    {
        if (i >= (unsigned int)Size())
            return m_End;
        return m_Begin + i;
    }

    /// @brief Finds the first occurrence of an element.
    T *Find(const T &o) const
    {
        for (T *t = m_Begin; t != m_End; ++t)
        {
            if (*t == o)
                return t;
        }
        return m_End;
    }

    /// @brief Checks if an element is present in the array.
    XBOOL IsHere(const T &o) const { return Find(o) != m_End; }

    /// @brief Gets the index of the first occurrence of an element.
    int GetPosition(const T &o) const
    {
        T *t = Find(o);
        return (t == m_End) ? -1 : (t - m_Begin);
    }

    /// @brief Swaps two elements in the array.
    void Swap(int pos1, int pos2)
    {
        T temp = *(m_Begin + pos1);
        *(m_Begin + pos1) = *(m_Begin + pos2);
        *(m_Begin + pos2) = temp;
    }

    /// @brief Swaps the contents of this array with another.
    void Swap(XSArray<T> &a)
    {
        XSwap(m_Begin, a.m_Begin);
        XSwap(m_End, a.m_End);
    }

    /// @brief A default comparison function for sorting.
    static int XCompare(const void *elem1, const void *elem2)
    {
        return (*(T *)elem1 > *(T *)elem2) ? 1 : ((*(T *)elem1 < *(T *)elem2) ? -1 : 0);
    }

    /// @brief Sorts the array using the C standard library's `qsort`.
    void Sort(VxSortFunc compare = XCompare)
    {
        if (Size() > 1)
            qsort(m_Begin, Size(), sizeof(T), compare);
    }

    /// @brief Sorts the array using bubble sort.
    void BubbleSort(VxSortFunc compare = XCompare)
    {
        if (!compare || Size() <= 1)
            return;
        XBOOL Noswap = TRUE;
        for (T *it1 = m_Begin + 1; it1 < m_End; it1++)
        {
            for (T *it2 = m_End - 1; it2 >= it1; it2--)
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

    /// @brief Returns an iterator to the beginning of the array.
    T *Begin() const { return m_Begin; }

    /// @brief STL-compatible begin() for range-for and algorithms.
    T *begin() { return m_Begin; }
    const T *begin() const { return m_Begin; }
    const T *cbegin() const { return m_Begin; }

    /// @brief Returns an iterator to the end of the array.
    T *End() const { return m_End; }

    /// @brief STL-compatible end() for range-for and algorithms.
    T *end() { return m_End; }
    const T *end() const { return m_End; }
    const T *cend() const { return m_End; }
    /// @brief Returns the number of elements in the array.
    int Size() const { return m_Begin ? (int)(m_End - m_Begin) : 0; }

    /// @brief Returns the memory occupied by the array in bytes.
    int GetMemoryOccupation(XBOOL addstatic = FALSE) const
    {
        return Size() * sizeof(T) + (addstatic ? sizeof(*this) : 0);
    }

protected:
    /// @name Internal Memory Management
    ///@{

    void XCopy(T *dest, T *start, T *end)
    {
        if (start == end)
            return;
        int size = ((XBYTE *)end - (XBYTE *)start);
        if (size > 0)
            memcpy(dest, start, size);
    }

    void XMove(T *dest, T *start, T *end)
    {
        if (start == end)
            return;
        int size = ((XBYTE *)end - (XBYTE *)start);
        if (size > 0)
            memmove(dest, start, size);
    }

    void XInsert(T *i, const T &o)
    {
        // Reallocation
        int oldSize = Size();
        int newsize = oldSize + 1;
        T *newdata = Allocate(newsize);

        int insertOffset = (oldSize > 0) ? (int)(i - m_Begin) : 0;

        // copy before insertion point
        if (insertOffset > 0)
        {
            XCopy(newdata, m_Begin, m_Begin + insertOffset);
        }

        // copy the new element
        T *insertionpoint = newdata + insertOffset;
        *(insertionpoint) = o;

        // copy after insertion point
        if (oldSize > insertOffset)
        {
            XCopy(insertionpoint + 1, m_Begin + insertOffset, m_End);
        }

        // New Pointers
        m_End = newdata + newsize;
        Free();
        m_Begin = newdata;
    }

#if VX_HAS_CXX11
    void XInsert(T *i, T &&o)
    {
        int oldSize = Size();
        int newsize = oldSize + 1;
        T *newdata = Allocate(newsize);

        int insertOffset = (oldSize > 0) ? (int)(i - m_Begin) : 0;

        if (insertOffset > 0)
        {
            XCopy(newdata, m_Begin, m_Begin + insertOffset);
        }

        T *insertionpoint = newdata + insertOffset;
        *(insertionpoint) = std::move(o);

        if (oldSize > insertOffset)
        {
            XCopy(insertionpoint + 1, m_Begin + insertOffset, m_End);
        }

        m_End = newdata + newsize;
        Free();
        m_Begin = newdata;
    }
#endif

    T *XRemove(T *i)
    {
        // Reallocation
        int oldSize = Size();
        int newsize = oldSize - 1;
        T *newdata = Allocate(newsize);

        int deleteOffset = (oldSize > 0) ? (int)(i - m_Begin) : 0;

        // copy before deletion point
        if (deleteOffset > 0)
        {
            XCopy(newdata, m_Begin, m_Begin + deleteOffset);
        }

        // copy after insertion point
        T *deletionpoint = newdata + deleteOffset;
        if (oldSize > (deleteOffset + 1))
        {
            XCopy(deletionpoint, m_Begin + deleteOffset + 1, m_End);
        }
        i = deletionpoint;

        // New Pointers
        m_End = newdata + newsize;
        Free();
        m_Begin = newdata;

        return i;
    }

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
    T *m_Begin; ///< @internal Pointer to the beginning of the array.
    T *m_End;   ///< @internal Pointer to the end of the array.
    ///@}
};

#endif // XSARRAY_H
