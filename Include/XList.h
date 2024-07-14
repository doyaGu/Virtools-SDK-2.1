#ifndef XLIST_H
#define XLIST_H

#include "VxMathDefines.h"
#include "XUtil.h"

#if VX_HAS_CXX11
#include <algorithm>
#endif

template <class T>
class XNode
{
public:
#if VX_HAS_CXX11
    XNode(XNode<T> &&e) VX_NOEXCEPT : m_Data(std::move(e.m_Data)), m_Next(e.m_Next), m_Prev(e.m_Prev)
    {
        e.m_Next = NULL;
        e.m_Prev = NULL;
    }
#endif

    T m_Data;
    XNode<T> *m_Next;
    XNode<T> *m_Prev;
};

/************************************************
Summary: Iterator on an XList.

Example:
Usage for iterating on a list :
    for (XListIt<T> it = list.Begin(); it != list.End(); ++it) {
        // Do whatever you want with *it, a reference on a T
    }


************************************************/
template <class T>
class XListIt
{
    typedef XNode<T> *tNode;

public:
    // Ctor
    XListIt() : m_Node(0) {}
    XListIt(XNode<T> *n) : m_Node(n) {}
    XListIt(const XListIt<T> &n) : m_Node(n.m_Node) {}
#if VX_HAS_CXX11
    XListIt(XListIt<T> &&n) VX_NOEXCEPT : m_Node(n.m_Node)
    {
        n.m_Node = NULL;
    }
#endif

    // Operators
    int operator==(const XListIt<T> &it) const { return m_Node == it.m_Node; }
    int operator!=(const XListIt<T> &it) const { return m_Node != it.m_Node; }

    /************************************************
    Summary: Returns a reference on the current element.
    ************************************************/
    T &operator*() const { return (*m_Node).m_Data; }

    /************************************************
    Summary: Go to the next element.
    ************************************************/
    XListIt<T> &operator++() // Prefixe
    {
        m_Node = tNode(m_Node->m_Next);
        return *this;
    }
    XListIt<T> operator++(int) // PostFixe
    {
        XListIt<T> tmp = *this;
        ++*this;
        return tmp;
    }

    /************************************************
    Summary: Go to the previous element.
    ************************************************/
    XListIt<T> &operator--()
    {
        m_Node = tNode(m_Node->m_Prev);
        return *this;
    }
    XListIt<T> operator--(int)
    {
        XListIt<T> tmp = *this;
        --*this;
        return tmp;
    }

    XListIt<T> operator+(int iOffset) const
    {
        XListIt<T> tmp = *this;
        while (iOffset--)
        {
            ++tmp;
        }
        return tmp;
    }

    XListIt<T> operator-(int iOffset) const
    {
        XListIt<T> tmp = *this;
        while (iOffset--)
        {
            --tmp;
        }
        return tmp;
    }

    XNode<T> *m_Node;
};

/************************************************
Summary: Doubly linked list.

Remarks:
    You can only create a list of elements which
    can be constructed from nothing (default constructor
    with no argument) and the element should also
    possess an operator =.


************************************************/
template <class T>
class XList
{
    typedef XNode<T> *tNode;

public:
    typedef XListIt<T> Iterator;

    /************************************************
    Summary: Constructors.

    Input Arguments:
        list: list to recopy in the new one.

    ************************************************/
    XList()
    {
#ifdef NO_VX_MALLOC
        m_Node = new XNode<T>;
#else
        m_Node = VxNew(XNode<T>);
#endif
        m_Node->m_Prev = m_Node;
        m_Node->m_Next = m_Node;
        m_Count = 0;
    }

    XList(const XList<T> &list)
    {
#ifdef NO_VX_MALLOC
        m_Node = new XNode<T>;
#else
        m_Node = VxNew(XNode<T>);
#endif
        m_Node->m_Prev = m_Node;
        m_Node->m_Next = m_Node;
        m_Count = 0;
        for (XListIt<T> it = list.Begin(); it != list.End(); ++it)
        {
            PushBack(*it);
        }
    }

#if VX_HAS_CXX11
    XList(XList<T> &&list) VX_NOEXCEPT
    {
        m_Node = list.m_Node;
        m_Count = list.m_Count;
        list.m_Node = NULL;
        list.m_Count = 0;
    }
#endif

    /************************************************
    Summary: Affectation operator.

    Remarks:
        The content of the list is entirely overwritten
    by the given one.
    ************************************************/
    XList &operator=(const XList<T> &list)
    {
        if (this != &list)
        {
            Clear();
            for (XListIt<T> it = list.Begin(); it != list.End(); ++it)
            {
                PushBack(*it);
            }
        }
        return *this;
    }

#if VX_HAS_CXX11
    XList &operator=(XList<T> &&list) VX_NOEXCEPT
    {
        if (this != &list)
        {
            Clear();
            m_Node = list.m_Node;
            m_Count = list.m_Count;
            list.m_Node = NULL;
            list.m_Count = 0;
        }
        return *this;
    }
#endif

    /************************************************
    Summary: Destructor.

    Remarks:
        Release the elements contained in the array. If
    you were storing pointers, you need first to iterate
    on the list and call delete on each pointer.
    ************************************************/
    ~XList()
    {
        Clear();
#ifdef NO_VX_MALLOC
        delete m_Node;
#else
        VxDelete< XNode<T> >(m_Node);
#endif
    }

    /************************************************
    Summary: Removes all the elements of a list.
    ************************************************/
    void Clear()
    {
        tNode tmp = XBegin();
        tNode del;
        while (tmp != XEnd())
        {
            del = tmp;
            tmp = tmp->m_Next;
#ifdef NO_VX_MALLOC
            delete del;
#else
            VxDelete<XNode<T> >(del);
#endif
        }
        m_Node->m_Prev = m_Node;
        m_Node->m_Next = m_Node;
        m_Count = 0;
    }

    /************************************************
    Summary: Returns the elements number.
    ************************************************/
    int Size() const
    {
        return m_Count;
    }

    /************************************************
    Summary: Returns a copy of the first element of an array.

    Remarks:
        No test are provided to see if there is an
    element.
    ************************************************/
    T Front() const
    {
        return XBegin()->m_Data;
    }

    /************************************************
    Summary: Returns a reference on the first element of an array.

    Remarks:
        No test are provided to see if there is an
    element.
    ************************************************/
    T &Front()
    {
        return XBegin()->m_Data;
    }

    /************************************************
    Summary: Returns a copy of the last element of an array.

    Remarks:
        No test are provided to see if there is an
    element.
    ************************************************/
    T Back() const
    {
        XListIt<T> tmp = End();
        return *(--tmp);
    }

    /************************************************
    Summary: Returns a reference on the last element of an array.

    Remarks:
        No test are provided to see if there is an
    element.
    ************************************************/
    T &Back()
    {
        XListIt<T> tmp = End();
        return *(--tmp);
    }

    /************************************************
    Summary: Inserts an element at the end of a list.

    Input Arguments:
        o: object to insert.
    ************************************************/
    void PushBack(const T &o)
    {
        XInsert(XEnd(), o);
    }

    /************************************************
    Summary: Inserts an element at the start of a list.

    Input Arguments:
        o: object to insert.
    ************************************************/
    void PushFront(const T &o)
    {
        XInsert(XBegin(), o);
    }

    /************************************************
    Summary: Inserts an element before another one.

    Input Arguments:
        i: iterator on the element to insert before.
        o: object to insert.
    ************************************************/
    void Insert(XListIt<T> &i, const T &o)
    {
        XInsert(i.m_Node, o);
    }

    /************************************************
    Summary: Removes an element at the end of a list.
    ************************************************/
    void PopBack()
    {
        XListIt<T> tmp = End();
        XRemove((--tmp).m_Node);
    }

    /************************************************
    Summary: Removes an element at the beginning of
    a list.
    ************************************************/
    void PopFront()
    {
        XRemove(XBegin());
    }

    /************************************************
    Summary: Finds an element.

    Input Arguments:
        o: object to find.
        start: iterator from which begin the search.
    Return Value : An iterator on the object found,
    End() if the object wasn't found.
    ************************************************/
    XListIt<T> Find(const T &o) const
    {
        m_Node->m_Data = o;
        XListIt<T> it = Begin();
        while (*it != o)
            ++it;
        return it;
    }
    XListIt<T> Find(const XListIt<T> &start, const T &o) const
    {
        m_Node->m_Data = o;
        XListIt<T> it = start;
        while (*it != o)
            ++it;
        return it;
    }

    /************************************************
    Summary: Test the presence of an element.

    Input Arguments:
        o: object to test.
    Return Value: TRUE if the object was found,
    otherwise FALSE.
    ************************************************/
    XBOOL IsHere(const T &o) const
    {
        XListIt<T> it = Find(o);
        if (it != End())
            return TRUE;
        return FALSE;
    }

    /************************************************
    Summary: Removes an element.

    Input Arguments:
        o: object to remove.
    Return Value: TRUE if the object was removed,
    otherwise FALSE.
    ************************************************/
    XBOOL Remove(const T &o)
    {
        XListIt<T> it = Find(o);
        if (it == End())
            return FALSE;
        else
        {
            Remove(it);
            return TRUE;
        }
    }

    /************************************************
    Summary: Removes an element.

    Input Arguments:
        i: iterator on the object to remove.
    Return Value: an iterator on the element following
    the removed one.
    ************************************************/
    XListIt<T> Remove(XListIt<T> &i)
    {
        return XRemove(i.m_Node);
    }

    /************************************************
    Summary: Returns an iterator on the first element.
    ************************************************/
    XListIt<T> Begin() const { return XBegin(); }

    /************************************************
    Summary: Returns an iterator after the last element.
    ************************************************/
    XListIt<T> End() const { return XEnd(); }

    /************************************************
    Summary: Swaps two lists.

    Input Arguments:
        o: second list to swap with.
    ************************************************/
    void Swap(XList<T> &a)
    {
        XSwap(m_Node, a.m_Node);
        XSwap(m_Count, a.m_Count);
    }

private:
    ///
    // Methods

    XNode<T> *XBegin() const { return tNode(m_Node->m_Next); }

    XNode<T> *XEnd() const { return tNode(m_Node); }

    XNode<T> *XInsert(XNode<T> *i, const T &o)
    {
#ifdef NO_VX_MALLOC
        XNode<T> *n = new XNode<T>;
#else
        XNode<T> *n = VxNew(XNode<T>);
#endif
        // Data
        n->m_Data = o;
        // Pointers
        n->m_Next = i;
        n->m_Prev = i->m_Prev;
        i->m_Prev->m_Next = n;
        i->m_Prev = n;
        m_Count++;

        return n;
    }

    XNode<T> *XRemove(XNode<T> *i)
    {
        XNode<T> *next = i->m_Next;
        XNode<T> *prev = i->m_Prev;
        prev->m_Next = next;
        next->m_Prev = prev;
        // we delete the old node
#ifdef NO_VX_MALLOC
        delete i;
#else
        VxDelete<XNode<T> >(i);
#endif
        m_Count--;
        // We return the element just after
        return next;
    }

    ///
    // Members

    XNode<T> *m_Node;

    int m_Count;
};

#endif // XLIST_H
