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
    XBitArray(int initialize = 1)
    {
        if (initialize < 1) initialize = 1;
        m_Size = (initialize << 5);
        m_Data = Allocate(initialize);
        Clear();
    }

    ~XBitArray()
    {
        Free();
    }

    // copy Ctor
    XBitArray(const XBitArray &a)
    {
        m_Size = a.m_Size;
        m_Data = Allocate(m_Size >> 5);
        memcpy(m_Data, a.m_Data, m_Size >> 3);
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
                memcpy(m_Data, a.m_Data, m_Size >> 3);
            }
            else
            {
                memcpy(m_Data, a.m_Data, m_Size >> 3);
            }
        }
        return *this;
    }

    // Reallocation if necessary
    void CheckSize(int n)
    {
        while (n >= m_Size)
        {
            int size = (m_Size >> 5);
            XDWORD *temp = Allocate(size + size);

            // Copy the old bits
            memcpy(temp, m_Data, size * sizeof(XDWORD));

            // Clear the new bits
            memset(temp + size, 0, size * sizeof(XDWORD));

            Free();

            m_Data = temp;
            m_Size += m_Size;
        }
    }

    void CheckSameSize(XBitArray &a)
    {
        if (m_Size < a.m_Size)
        {
            int size = a.m_Size >> 5;
            int oldsize = m_Size >> 5;
            XDWORD *temp = Allocate(size);
            // Copy the old bits
            memcpy(temp, m_Data, oldsize * sizeof(XDWORD));

            // Clear the new bits
            memset(temp + oldsize, 0, (size - oldsize) * sizeof(XDWORD));

            Free();

            m_Data = temp;
            m_Size = a.m_Size;
        }
    }

    // Summary: Returns if the n-th bit is set to 1
    int IsSet(int n)
    {
        if (n >= m_Size)
            return 0; // Out of range
        return (m_Data[n >> 5] & (1 << (n & 31))); // Allocated after the first DWORD
    }

    // Summary: Returns if the n-th bit is set to 1
    int operator[](int n)
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
        if (n < m_Size)
        {
            m_Data[n >> 5] |= 1 << (n & 31);
        }
        else
        {
            // we have to reallocate
            CheckSize(n);
            m_Data[n >> 5] |= 1 << (n & 31);
        }
    }

    // Summary: Sets the n-th bit to 1 and return 0 if it was set, 1 otherwise
    int TestSet(int n)
    {
        if (n < m_Size)
        {
            int pos = n >> 5;
            int mask = 1 << (n & 31);

            if (m_Data[pos] & mask)
                return 0;
            m_Data[pos] |= mask;
            return 1;
        }
        else
        {
            // we have to reallocate
            CheckSize(n);
            m_Data[n >> 5] |= 1 << (n & 31);
            return 1;
        }
    }

    // Summary: Sets the n-th bit to 0
    void Unset(int n)
    {
        if (n < m_Size)
        {
            m_Data[n >> 5] &= ~(1 << (n & 31));
        }
    }

    // Summary: Sets the n-th bit to 0 and return 1 if it was set, 0 otherwise
    int TestUnset(int n)
    {
        if (n < m_Size)
        {
            int pos = n >> 5;
            int mask = 1 << (n & 31);
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
        memset(m_Data, 0, (m_Size >> 3));
    }

    // Summary: Sets all bits of the array to 1
    // Warning : this functions sets all the allocated bits, not only the
    // used bits
    void Fill()
    {
        memset(m_Data, 0xff, (m_Size >> 3));
    }

    // Summary: Performs a binary AND with another array
    void And(XBitArray &a)
    {
        int size = a.m_Size >> 5;
        int i = 0;
        for (; i < size; ++i)
        {
            m_Data[i] &= a.m_Data[i];
        }
        // clear the remaining bytes
        int rsize = m_Size >> 5;
        for (; i < rsize; ++i)
        {
            m_Data[i] = 0;
        }
    }

    // Summary: subtract bits from another bitarray
    XBitArray &operator-=(XBitArray &a)
    {
        int size = a.m_Size >> 5;
        int i = 0;
        for (; i < size; ++i)
        {
            m_Data[i] &= ~a.m_Data[i];
        }
        return *this;
    }

    // Summary: Returns TRUE if at least one common bit is set in two arrays
    XBOOL CheckCommon(XBitArray &a)
    {
        int size = a.m_Size >> 5;
        for (int i = 0; i < size; ++i)
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
            int count = 0;
            for (int i = 0; i < (m_Size >> 5); i++)
            {
                for (int j = 0; j < 32; j++)
                {
                    if (m_Data[i] & (1 << j))
                        buffer[count] = '1';
                    else
                        buffer[count] = '0';
                    count++;
                }
            }
            buffer[m_Size] = 0;
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
        return (XDWORD *)VxNew(sizeof(XDWORD) * size);
    }

    void Free()
    {
        VxDelete(m_Data);
    }

    // the array itself {secret}
    XDWORD *m_Data;
    // real size already allocated {secret}
    int m_Size;
};

#endif // BITARRAY_H
