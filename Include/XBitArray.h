#ifndef XBITARRAY_H
#define XBITARRAY_H

#include "XUtil.h"

/************************************************
{filename:XBitArray}
Summary: Set of bit flags.

Remarks:

    o This class  define a set of bit flags that may be treated as a virtual array but are stored in an efficient manner.
    o The class has methods to set, clear and return the i-th bit, resize the array, etc.
************************************************/
class XBitArray
{
public:
    explicit XBitArray(int initialize = 1)
    {
        if (initialize < 1) initialize = 1;
        m_Size = (initialize << 5);
        m_Data = Allocate(initialize);
        Clear();
    }

    // Copy Ctor
    XBitArray(const XBitArray &a)
    {
        m_Size = a.m_Size;
        m_Data = Allocate(m_Size >> 5);
        if (m_Data && a.m_Data)
            memcpy(m_Data, a.m_Data, m_Size >> 3);
    }

#if VX_HAS_CXX11
    // Move Ctor
    XBitArray(XBitArray &&a) VX_NOEXCEPT
    {
        m_Data = a.m_Data;
        m_Size = a.m_Size;
        a.m_Data = NULL;
        a.m_Size = 0;
    }
#endif

    ~XBitArray()
    {
        Free();
    }

    // operator =
    XBitArray &operator=(const XBitArray &a)
    {
        if (this != &a)
        {
            if (m_Size != a.m_Size)
            {
                Free();
                m_Size = a.m_Size;
                m_Data = Allocate(m_Size >> 5);
            }

            memcpy(m_Data, a.m_Data, m_Size >> 3);
        }
        return *this;
    }

#if VX_HAS_CXX11
    // operator =
    XBitArray &operator=(XBitArray &&a) VX_NOEXCEPT
    {
        if (this != &a)
        {
            Free();
            m_Data = a.m_Data;
            m_Size = a.m_Size;
            a.m_Data = NULL;
            a.m_Size = 0;
        }
        return *this;
    }
#endif

    // Reallocation if necessary
    void CheckSize(int n)
    {
        while (n >= m_Size)
        {
            int dwords = (m_Size >> 5);
            int newDwords = dwords ? dwords * 2 : 1;
            XDWORD *temp = Allocate(newDwords);

            if (temp && m_Data)
            {
                // Copy the old bits
                memcpy(temp, m_Data, dwords * sizeof(XDWORD));
                // Clear the new bits
                memset(temp + dwords, 0, (newDwords - dwords) * sizeof(XDWORD));
            }

            Free();

            m_Data = temp;
            m_Size = newDwords * 32;
        }
    }

    void CheckSameSize(XBitArray &a)
    {
        if (m_Size < a.m_Size)
        {
            int newDwords = a.m_Size >> 5;
            int dwords = m_Size >> 5;
            XDWORD *temp = Allocate(newDwords);

            if (temp && m_Data)
            {
                // Copy the old bits
                memcpy(temp, m_Data, dwords * sizeof(XDWORD));
                // Clear the new bits
                memset(temp + dwords, 0, (newDwords - dwords) * sizeof(XDWORD));
            }

            Free();

            m_Data = temp;
            m_Size = a.m_Size;
        }
    }

    // Summary: Returns if the n-th bit is set to 1
    int IsSet(int n) const
    {
        if (n >= m_Size)
            return 0;                              // Out of range
        return (m_Data[n >> 5] & (1U << (n & 31))); // Allocated after the first DWORD
    }

    // Summary: Returns if the n-th bit is set to 1
    int operator[](int n) const
    {
        return IsSet(n);
    }

    // Summary: Appends the bitcount first bits of integer v to the array
    void AppendBits(int n, int v, int bitcount)
    {
        int mask = 1;
        bitcount += n;
        for (int i = n; i < bitcount; ++i, mask <<= 1)
        {
            if (mask & v)
                Set(i);
            else
                Unset(i);
        }
    }

    // Summary: Sets the n-th bit to 1
    void Set(int n)
    {
        if (n < 0) return;
        if (n >= m_Size)
            CheckSize(n);
        m_Data[n >> 5] |= (1U << (n & 31));
    }

    // Summary: Sets the n-th bit to 1 and return 0 if it was set, 1 otherwise
    int TestSet(int n)
    {
        if (n < 0) return 0;
        if (n >= m_Size)
        {
            CheckSize(n);
            m_Data[n >> 5] |= (1U << (n & 31));
            return 1;
        }
        else
        {
            int pos = n >> 5;
            unsigned int mask = 1U << (n & 31);
            if (m_Data[pos] & mask)
                return 0;
            m_Data[pos] |= mask;
            return 1;
        }
    }

    // Summary: Sets the n-th bit to 0
    void Unset(int n)
    {
        if (n < m_Size && n >= 0)
        {
            m_Data[n >> 5] &= ~(1U << (n & 31));
        }
    }

    // Summary: Sets the n-th bit to 0 and return 1 if it was set, 0 otherwise
    int TestUnset(int n)
    {
        if (n < m_Size && n >= 0)
        {
            int pos = n >> 5;
            unsigned int mask = 1U << (n & 31);
            if (m_Data[pos] & mask)
            {
                m_Data[pos] &= ~mask;
                return 1;
            }
            return 0;
        }
        else
            return 0;
    }

    // Summary: Returns the number of bits
    int Size() const
    {
        return m_Size;
    }

    // Summary: Resets the array
    void Clear()
    {
        if (m_Data)
            memset(m_Data, 0, (m_Size >> 3));
    }

    // Summary: Sets all bits of the array to 1
    // Warning : this functions sets all the allocated bits, not only the
    // used bits
    void Fill()
    {
        if (m_Data)
            memset(m_Data, 0xff, (m_Size >> 3));
    }

    // Summary: Performs a binary AND with another array
    void And(XBitArray &a)
    {
        int dwords1 = m_Size >> 5;
        int dwords2 = a.m_Size >> 5;
        int dwords = XMin(dwords1, dwords2);
        for (int i = 0; i < dwords; ++i)
        {
            m_Data[i] &= a.m_Data[i];
        }
        // clear the remaining bytes in this array
        for (int i = dwords; i < dwords1; ++i)
        {
            m_Data[i] = 0;
        }
    }

    // Summary: subtract bits from another bitarray
    XBitArray &operator-=(XBitArray &a)
    {
        int dwords1 = m_Size >> 5;
        int dwords2 = a.m_Size >> 5;
        int dwords = XMin(dwords1, dwords2);
        for (int i = 0; i < dwords; ++i)
        {
            m_Data[i] &= ~a.m_Data[i];
        }
        return *this;
    }

    // Summary: Returns TRUE if at least one common bit is set in two arrays
    XBOOL CheckCommon(XBitArray &a)
    {
        int dwords1 = m_Size >> 5;
        int dwords2 = a.m_Size >> 5;
        int dwords = XMin(dwords1, dwords2);
        for (int i = 0; i < dwords; ++i)
        {
            if (m_Data[i] & a.m_Data[i])
                return TRUE;
        }
        return FALSE;
    }

    // Summary: Performs a binary OR with another array
    void Or(XBitArray &a)
    {
        CheckSameSize(a);
        int size = a.m_Size >> 5;
        for (int i = 0; i < size; ++i)
        {
            m_Data[i] |= a.m_Data[i];
        }
    }

    // Summary: Performs a binary XOR with another array
    void XOr(XBitArray &a)
    {
        CheckSameSize(a);
        int size = a.m_Size >> 5;
        for (int i = 0; i < size; ++i)
        {
            m_Data[i] ^= a.m_Data[i];
        }
    }

    // Summary: Inverts all bits value in the array
    void Invert()
    {
        int size = m_Size >> 5;
        for (int i = 0; i < size; ++i)
        {
            m_Data[i] = ~m_Data[i];
        }
    }

    // Summary: Returns the number of bits set
    int BitSet()
    {
        int set = 0;
        int size = m_Size >> 5;
        for (int i = 0; i < size; ++i)
        {
            int mask = 1;
            for (int j = 0; j < 32; ++j)
            {
                if (m_Data[i] & mask)
                    ++set;
                mask <<= 1;
            }
        }
        return set;
    }

    // Summary: Returns the position of the n-th set(1) bit
    int GetSetBitPosition(int n)
    {
        int set = 0;
        int pos = 0;
        int size = m_Size >> 5;
        for (int i = 0; i < size; ++i)
        {
            int mask = 1;
            for (int j = 0; j < 32; ++j, ++pos)
            {
                if (m_Data[i] & mask)
                {
                    if (set == n)
                        return pos;
                    ++set;
                }
                mask <<= 1;
            }
        }
        return -1;
    }

    // Summary: Returns the position of the n-th unset(0) bit
    int GetUnsetBitPosition(int n)
    {
        int unset = 0;
        int pos = 0;
        int size = m_Size >> 5;
        for (int i = 0; i < size; ++i)
        {
            int mask = 1;
            for (int j = 0; j < 32; ++j, ++pos)
            {
                if (!(m_Data[i] & mask))
                {
                    if (unset == n)
                        return pos;
                    ++unset;
                }
                mask <<= 1;
            }
        }
        // We haven't found an unsetted bit yet : we reallocate
        CheckSize(pos);
        return pos;
    }

    char *ConvertToString(char *buffer)
    {
        if (buffer)
        {
            for (int i = 0; i < m_Size; i++)
            {
                if (m_Data[i >> 5] & (1U << (i & 31)))
                    buffer[i] = '1';
                else
                    buffer[i] = '0';
            }
            buffer[m_Size] = '\0';
        }
        return buffer;
    }

    // Summary: Returns the occupied size in memory in bytes
    int GetMemoryOccupation(XBOOL addstatic = FALSE) const
    {
        return (m_Size >> 5) * sizeof(XDWORD) + (addstatic ? sizeof(*this) : 0);
    }

private:
    XDWORD *Allocate(int size)
    {
#ifdef NO_VX_MALLOC
        return new XDWORD[size];
#else
        return (XDWORD *)VxMalloc(sizeof(XDWORD) * size);
#endif
    }

    void Free()
    {
#ifdef NO_VX_MALLOC
        delete[] m_Data;
#else
        VxFree(m_Data);
#endif
        m_Data = NULL;
    }

    // the array itself {secret}
    XDWORD *m_Data;
    // real size already allocated {secret}
    int m_Size;
};

#endif // BITARRAY_H
