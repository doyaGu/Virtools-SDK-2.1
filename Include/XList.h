#ifndef XLIST_H
#define XLIST_H

#include "VxMathDefines.h"
#include "XUtil.h"

#if VX_HAS_CXX11
#include <initializer_list>
#include <utility>
#endif

/**
 * @class XNode
 * @brief Internal node structure for the XList doubly linked list.
 * @internal
 * @tparam T The type of data stored in the node.
 */
template <class T>
class XNode
{
public:
    T m_Data;         ///< The data element stored in the node.
    XNode<T> *m_Next; ///< Pointer to the next node in the list.
    XNode<T> *m_Prev; ///< Pointer to the previous node in the list.
};

/**
 * @class XListIt
 * @brief An iterator for traversing an XList.
 *
 * @tparam T The type of the elements in the list.
 *
 * @example
 * Usage for iterating through a list:
 * @code
 * XList<MyType> list;
 * // ... populate list ...
 * for (XListIt<MyType> it = list.Begin(); it != list.End(); ++it) {
 *     // Do something with *it, which is a reference to a MyType object.
 * }
 * @endcode
 */
template <class T>
class XListIt
{
public:
    /// @brief Default constructor. Initializes to a null iterator.
    XListIt() : m_Node(NULL) {}

    /**
     * @brief Constructs an iterator from a list node.
     * @internal
     * @param n A pointer to an XNode.
     */
    XListIt(XNode<T> *n) : m_Node(n) {}

    /**
     * @brief Copy constructor.
     * @param n The iterator to copy.
     */
    XListIt(const XListIt<T> &n) : m_Node(n.m_Node) {}

    /// @brief Equality comparison operator.
    int operator==(const XListIt<T> &it) const { return m_Node == it.m_Node; }
    /// @brief Inequality comparison operator.
    int operator!=(const XListIt<T> &it) const { return m_Node != it.m_Node; }

    /**
     * @brief Dereferences the iterator to access the current element.
     * @return A reference to the element at the current position.
     */
    T &operator*() const { return m_Node->m_Data; }

    /**
     * @brief Pre-increment operator. Advances the iterator to the next element.
     */
    XListIt<T> &operator++()
    {
        m_Node = m_Node->m_Next;
        return *this;
    }

    /**
     * @brief Post-increment operator. Advances the iterator to the next element.
     */
    XListIt<T> operator++(int)
    {
        XListIt<T> tmp = *this;
        ++*this;
        return tmp;
    }

    /**
     * @brief Pre-decrement operator. Moves the iterator to the previous element.
     */
    XListIt<T> &operator--()
    {
        m_Node = m_Node->m_Prev;
        return *this;
    }

    /**
     * @brief Post-decrement operator. Moves the iterator to the previous element.
     */
    XListIt<T> operator--(int)
    {
        XListIt<T> tmp = *this;
        --*this;
        return tmp;
    }

    /**
     * @brief Advances the iterator by a specified offset.
     * @param iOffset The number of elements to advance.
     * @return A new iterator at the advanced position.
     */
    XListIt<T> operator+(int iOffset) const
    {
        XListIt<T> tmp = *this;
        while (iOffset--)
            ++tmp;
        return tmp;
    }

    /**
     * @brief Moves the iterator backward by a specified offset.
     * @param iOffset The number of elements to move backward.
     * @return A new iterator at the new position.
     */
    XListIt<T> operator-(int iOffset) const
    {
        XListIt<T> tmp = *this;
        while (iOffset--)
            --tmp;
        return tmp;
    }

    /// @brief The internal node the iterator is pointing to.
    XNode<T> *m_Node;
};

/**
 * @class XList
 * @brief A template class for a doubly linked list.
 *
 * @tparam T The type of elements to be stored.
 *
 * @remarks
 * This list implementation requires that the element type `T` has a default
 * constructor and a copy assignment operator.
 */
template <class T>
class XList
{
public:
    typedef XListIt<T> Iterator; ///< A typedef for the list's iterator.

    /**
     * @brief Default constructor. Creates an empty list.
     */
    XList()
    {
        m_Node = new XNode<T>;
        m_Node->m_Prev = m_Node;
        m_Node->m_Next = m_Node;
        m_Count = 0;
    }

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @remark Leaves the source list empty.
     */
    XList(XList<T> &&list) VX_NOEXCEPT : XList()
    {
        Swap(list);
    }

    /**
     * @brief Constructs from an initializer_list (C++11).
     */
    XList(std::initializer_list<T> init) : XList()
    {
        for (typename std::initializer_list<T>::const_iterator it = init.begin(); it != init.end(); ++it)
        {
            PushBack(*it);
        }
    }
#endif

    /**
     * @brief Copy constructor. Creates a deep copy of another list.
     * @param list The list to copy from.
     */
    XList(const XList<T> &list)
    {
        m_Node = new XNode<T>;
        m_Node->m_Prev = m_Node;
        m_Node->m_Next = m_Node;
        m_Count = 0;
        for (Iterator it = list.Begin(); it != list.End(); ++it)
        {
            PushBack(*it);
        }
    }

    /**
     * @brief Assignment operator. Replaces the list's content with a copy of another list.
     * @param list The list to assign from.
     * @return A reference to this list.
     */
    XList &operator=(const XList<T> &list)
    {
        if (&list != this)
        {
            Clear();
            for (Iterator it = list.Begin(); it != list.End(); ++it)
            {
                PushBack(*it);
            }
        }
        return *this;
    }

#if VX_HAS_CXX11
    /**
     * @brief Move assignment operator (C++11).
     * @remark Leaves the source list empty.
     */
    XList &operator=(XList<T> &&list) VX_NOEXCEPT
    {
        Swap(list);
        list.Clear();
        return *this;
    }

    /**
     * @brief Assigns from an initializer_list (C++11).
     */
    XList &operator=(std::initializer_list<T> init)
    {
        Clear();
        for (typename std::initializer_list<T>::const_iterator it = init.begin(); it != init.end(); ++it)
        {
            PushBack(*it);
        }
        return *this;
    }
#endif

    /**
     * @brief Destructor.
     * @remarks Frees all nodes in the list. If the list stores pointers, it is the
     * user's responsibility to delete the pointed-to objects before destroying the list.
     */
    ~XList()
    {
        Clear();
        delete m_Node;
    }

    /**
     * @brief Removes all elements from the list.
     */
    void Clear()
    {
        XNode<T> *tmp = XBegin();
        while (tmp != XEnd())
        {
            XNode<T> *del = tmp;
            tmp = tmp->m_Next;
            delete del;
        }
        m_Node->m_Prev = m_Node;
        m_Node->m_Next = m_Node;
        m_Count = 0;
    }

    /**
     * @brief Returns TRUE if the list contains no elements.
     */
    XBOOL IsEmpty() const { return m_Count == 0; }

    /**
     * @brief Returns the number of elements in the list.
     */
    int Size() const { return m_Count; }

    /**
     * @brief Returns a copy of the first element.
     * @remarks Behavior is undefined if the list is empty.
     */
    T Front() const { return XBegin()->m_Data; }

    /**
     * @brief Returns a reference to the first element.
     * @remarks Behavior is undefined if the list is empty.
     */
    T &Front() { return XBegin()->m_Data; }

    /**
     * @brief Returns a copy of the last element.
     * @remarks Behavior is undefined if the list is empty.
     */
    T Back() const
    {
        Iterator tmp = End();
        return *(--tmp);
    }

    /**
     * @brief Returns a reference to the last element.
     * @remarks Behavior is undefined if the list is empty.
     */
    T &Back()
    {
        Iterator tmp = End();
        return *(--tmp);
    }

    /**
     * @brief Adds an element to the end of the list.
     * @param o The element to add.
     */
    void PushBack(const T &o)
    {
        XInsert(XEnd(), o);
    }

#if VX_HAS_CXX11
    /**
     * @brief Adds an element to the end of the list (move) (C++11).
     */
    void PushBack(T &&o)
    {
        XInsert(XEnd(), std::move(o));
    }

    /**
     * @brief Emplaces an element at the end of the list (C++11).
     */
    template <class... Args>
    void EmplaceBack(Args &&...args)
    {
        T tmp(std::forward<Args>(args)...);
        XInsert(XEnd(), std::move(tmp));
    }
#endif

    /**
     * @brief Adds an element to the beginning of the list.
     * @param o The element to add.
     */
    void PushFront(const T &o)
    {
        XInsert(XBegin(), o);
    }

#if VX_HAS_CXX11
    /**
     * @brief Adds an element to the beginning of the list (move) (C++11).
     */
    void PushFront(T &&o)
    {
        XInsert(XBegin(), std::move(o));
    }

    /**
     * @brief Emplaces an element at the beginning of the list (C++11).
     */
    template <class... Args>
    void EmplaceFront(Args &&...args)
    {
        T tmp(std::forward<Args>(args)...);
        XInsert(XBegin(), std::move(tmp));
    }
#endif

    /**
     * @brief Inserts an element before the specified position.
     * @param i An iterator indicating the position to insert before.
     * @param o The element to insert.
     */
    void Insert(Iterator &i, const T &o)
    {
        XInsert(i.m_Node, o);
    }

#if VX_HAS_CXX11
    /**
     * @brief Inserts an element before the specified position (move) (C++11).
     */
    void Insert(Iterator &i, T &&o)
    {
        XInsert(i.m_Node, std::move(o));
    }

    /**
     * @brief Emplaces an element before the specified position (C++11).
     */
    template <class... Args>
    void Emplace(Iterator &i, Args &&...args)
    {
        T tmp(std::forward<Args>(args)...);
        XInsert(i.m_Node, std::move(tmp));
    }
#endif

    /**
     * @brief Removes the last element of the list.
     * @remarks Behavior is undefined if the list is empty.
     */
    void PopBack()
    {
        Iterator tmp = End();
        XRemove((--tmp).m_Node);
    }

    /**
     * @brief Removes the first element of the list.
     * @remarks Behavior is undefined if the list is empty.
     */
    void PopFront()
    {
        XRemove(XBegin());
    }

    /**
     * @brief Finds the first occurrence of an element.
     * @param o The element to find.
     * @return An iterator to the first found element, or `End()` if not found.
     */
    Iterator Find(const T &o) const
    {
        m_Node->m_Data = o; // Sentinel for search
        Iterator it = Begin();
        while (*it != o)
            ++it;
        return it;
    }

    /**
     * @brief Finds the first occurrence of an element starting from a specific position.
     * @param start The iterator to start the search from.
     * @param o The element to find.
     * @return An iterator to the first found element, or `End()` if not found.
     */
    Iterator Find(const Iterator &start, const T &o) const
    {
        m_Node->m_Data = o; // Sentinel for search
        Iterator it = start;
        while (*it != o)
            ++it;
        return it;
    }

    /**
     * @brief Checks if an element is present in the list.
     * @param o The element to check for.
     * @return TRUE if the element is found, FALSE otherwise.
     */
    XBOOL IsHere(const T &o) const
    {
        return Find(o) != End();
    }

    /**
     * @brief Removes the first occurrence of a specific element.
     * @param o The element to remove.
     * @return TRUE if an element was removed, FALSE otherwise.
     */
    XBOOL Remove(const T &o)
    {
        Iterator it = Find(o);
        if (it == End())
            return FALSE;
        Remove(it);
        return TRUE;
    }

    /**
     * @brief Removes an element at a specified position.
     * @param i An iterator pointing to the element to remove.
     * @return An iterator to the element that followed the removed element.
     */
    Iterator Remove(Iterator &i)
    {
        return XRemove(i.m_Node);
    }

    /**
     * @brief Returns an iterator to the beginning of the list.
     */
    Iterator Begin() const { return XBegin(); }

    /**
     * @brief Returns an iterator to the position after the last element.
     */
    Iterator End() const { return XEnd(); }

#if VX_HAS_CXX11
    /** @brief STL-compatible begin/end for range-for (C++11). */
    Iterator begin() const { return Begin(); }
    Iterator end() const { return End(); }
    Iterator cbegin() const { return Begin(); }
    Iterator cend() const { return End(); }
#endif

    /**
     * @brief Swaps the contents of this list with another.
     * @param a The other list to swap with.
     */
    void Swap(XList<T> &a)
    {
        XSwap(m_Node, a.m_Node);
        XSwap(m_Count, a.m_Count);
    }

private:
    /// @name Internal Methods
    ///@{

    XNode<T> *XBegin() const { return m_Node->m_Next; }
    XNode<T> *XEnd() const { return m_Node; }

    XNode<T> *XInsert(XNode<T> *i, const T &o)
    {
        XNode<T> *n = new XNode<T>;
        n->m_Data = o;
        n->m_Next = i;
        n->m_Prev = i->m_Prev;
        i->m_Prev->m_Next = n;
        i->m_Prev = n;
        m_Count++;
        return n;
    }

#if VX_HAS_CXX11
    XNode<T> *XInsert(XNode<T> *i, T &&o)
    {
        XNode<T> *n = new XNode<T>;
        n->m_Data = std::move(o);
        n->m_Next = i;
        n->m_Prev = i->m_Prev;
        i->m_Prev->m_Next = n;
        i->m_Prev = n;
        m_Count++;
        return n;
    }
#endif

    XNode<T> *XRemove(XNode<T> *i)
    {
        XNode<T> *next = i->m_Next;
        XNode<T> *prev = i->m_Prev;
        prev->m_Next = next;
        next->m_Prev = prev;
        delete i;
        m_Count--;
        return next;
    }

    ///@}

    /// @name Members
    ///@{
    XNode<T> *m_Node; ///< The sentinel node. `m_Node->m_Next` is the head, `m_Node->m_Prev` is the tail.
    int m_Count;      ///< The number of elements in the list.
    ///@}
};

#endif // XLIST_H
