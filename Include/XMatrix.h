#ifndef XSMATRIX_H
#define XSMATRIX_H

#include "VxMathDefines.h"
#include "XUtil.h"

// Template class Describing a 2D Matrix of variable width and height.
template <class T>
class XMatrix
{
public:
    explicit XMatrix(int iWidth = 0, int iHeight = 0) : m_Data(NULL), m_Width(0), m_Height(0)
    {
        Allocate(iWidth, iHeight);
    }

#if VX_HAS_CXX11
    XMatrix(XMatrix<T> &&m) VX_NOEXCEPT : m_Data(m.m_Data), m_Width(m.m_Width), m_Height(m.m_Height)
    {
        m.m_Data = NULL;
        m.m_Width = 0;
        m.m_Height = 0;
    }
#endif

    ~XMatrix()
    {
        Clear();
    }

#if VX_HAS_CXX11
    XMatrix<T> &operator=(XMatrix<T> &&m) VX_NOEXCEPT
    {
        if (this != &m)
        {
            Clear();
            m_Data = m.m_Data;
            m_Width = m.m_Width;
            m_Height = m.m_Height;
            m.m_Data = NULL;
            m.m_Width = 0;
            m.m_Height = 0;
        }
        return *this;
    }
#endif

    ///
    // Accessors

    // Returns the width of the matrix (Number of columns)
    int GetWidth() const
    {
        return m_Width;
    }

    // Returns the height of the matrix (Number of rows)
    int GetHeight() const
    {
        return m_Height;
    }

    // Returns the memory taken by the matrix in bytes
    int Size() const
    {
        return m_Width * m_Height * sizeof(T);
    }

    // Free the memory taken by the matrix.
    void Clear()
    {
#ifdef NO_VX_MALLOC
        delete[] m_Data;
#else
        VxDeallocate<T>(m_Data, iWidth * iHeight);
#endif
        m_Data = NULL;
        m_Width = 0;
        m_Height = 0;
    }

    // Creation of the matrix (automatically calls Clear)
    void Create(int iWidth, int iHeight)
    {
        Clear();
        Allocate(iWidth, iHeight);
    }

    // Access to an element of the matrix
    const T &operator()(const int iX, const int iY) const
    {
        XASSERT(iX < m_Width);
        XASSERT(iY < m_Height);
        return m_Data[iY * m_Width + iX];
    }

    T &operator()(const int iX, const int iY)
    {
        XASSERT(iX < m_Width);
        XASSERT(iY < m_Height);
        return m_Data[iY * m_Width + iX];
    }

private:
    // allocate the space for the matrix
    void Allocate(int iWidth, int iHeight)
    {
        int count = iWidth * iHeight;
        if (count)
        {
#ifdef NO_VX_MALLOC
            m_Data	= new T[count];
#else
            m_Data = VxAllocate<T>(count);
#endif
            XASSERT(m_Data); // No more free space ???
            m_Width = iWidth;
            m_Height = iHeight;
        }
    }

    ///
    // members

    // data
    T *m_Data;

    ///
    // dimensions

    // width
    int m_Width;
    // height
    int m_Height;
};

#endif // XSMATRIX_H