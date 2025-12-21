#ifndef XBITARRAY_H
#define XBITARRAY_H

#include "XUtil.h"

#if VX_HAS_CXX11
#include <initializer_list>
#include <utility>
#endif

/**
 * @class XBitArray
 * @brief An efficient class for managing a set of bit flags.
 *
 * @remarks
 * This class provides a way to handle a large number of boolean flags
 * by treating them as a virtual array of bits, but storing them compactly
 * in an array of integers. The array automatically resizes itself as needed
 * when bits beyond its current capacity are accessed.
 *
 * It offers methods to set, clear, and test individual bits, as well as
 * perform bitwise logical operations (AND, OR, XOR) with other XBitArray instances.
 */
class XBitArray
{
public:
    /**
     * @brief Constructs a XBitArray.
     * @param initialize The initial number of 32-bit `XDWORD`s to allocate.
     *                   Defaults to 1, creating an array with a capacity of 32 bits.
     */
    explicit XBitArray(int initialize = 1)
    {
        if (initialize < 1)
            initialize = 1;
        m_Size = (initialize << 5); // size in bits
        m_Data = Allocate(initialize);
        Clear();
    }

    /**
     * @brief Copy constructor. Creates a deep copy of another XBitArray.
     * @param a The XBitArray to copy from.
     */
    XBitArray(const XBitArray &a)
    {
        m_Size = a.m_Size;
        m_Data = Allocate(m_Size >> 5);
        if (m_Data && a.m_Data)
            memcpy(m_Data, a.m_Data, m_Size >> 3);
    }

#if VX_HAS_CXX11
    /**
     * @brief Constructs a bit array and sets the given bit indices (C++11).
     * @param setBits Bit indices to set to 1.
     */
    XBitArray(std::initializer_list<int> setBits) : m_Data(NULL), m_Size(0)
    {
        int maxBit = -1;
        for (int b : setBits)
        {
            if (b > maxBit)
                maxBit = b;
        }

        int dwords = (maxBit >= 0) ? ((maxBit >> 5) + 1) : 1;
        m_Size = (dwords << 5);
        m_Data = Allocate(dwords);
        Clear();
        for (int b : setBits)
        {
            Set(b);
        }
    }
#endif

#if VX_HAS_CXX11
    /**
     * @brief Move constructor (C++11).
     * @param a The XBitArray to move from.
     */
    XBitArray(XBitArray &&a) VX_NOEXCEPT
    {
        m_Data = a.m_Data;
        m_Size = a.m_Size;
        a.m_Data = NULL;
        a.m_Size = 0;
    }
#endif

    /**
     * @brief Destructor. Frees the allocated memory.
     */
    ~XBitArray()
    {
        Free();
    }

    /**
     * @brief Assignment operator.
     * @param a The XBitArray to assign from.
     * @return A reference to this object.
     */
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
            if (m_Data && a.m_Data)
                memcpy(m_Data, a.m_Data, m_Size >> 3);
        }
        return *this;
    }

#if VX_HAS_CXX11
    /**
     * @brief Assignment from initializer list of bit indices (C++11).
     * @param setBits Bit indices to set to 1.
     * @return A reference to this object.
     */
    XBitArray &operator=(std::initializer_list<int> setBits)
    {
        int maxBit = -1;
        for (int b : setBits)
        {
            if (b > maxBit)
                maxBit = b;
        }

        int dwords = (maxBit >= 0) ? ((maxBit >> 5) + 1) : 1;
        int newSize = (dwords << 5);
        if (m_Size != newSize)
        {
            Free();
            m_Size = newSize;
            m_Data = Allocate(dwords);
        }
        Clear();
        for (int b : setBits)
        {
            Set(b);
        }
        return *this;
    }
#endif

    /**
     * @brief Swaps the contents of this array with another.
     */
    void Swap(XBitArray &a) VX_NOEXCEPT
    {
        XSwap(m_Data, a.m_Data);
        XSwap(m_Size, a.m_Size);
    }

#if VX_HAS_CXX11
    /**
     * @brief Move assignment operator (C++11).
     * @param a The XBitArray to move from.
     * @return A reference to this object.
     */
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

    /**
     * @brief Ensures the array is large enough to hold a specific bit index.
     * @remarks If the index `n` is out of bounds, the array's capacity is doubled
     * until it is large enough. New bits are initialized to 0.
     * @param n The bit index to check for.
     */
    void CheckSize(int n)
    {
        while (n >= m_Size)
        {
            int dwords = (m_Size >> 5);
            int newDwords = dwords ? dwords * 2 : 1;
            XDWORD *temp = Allocate(newDwords);

            if (temp && m_Data)
            {
                memcpy(temp, m_Data, dwords * sizeof(XDWORD));
                memset(temp + dwords, 0, (newDwords - dwords) * sizeof(XDWORD));
            }

            Free();

            m_Data = temp;
            m_Size = newDwords * 32;
        }
    }

    /**
     * @brief Ensures this array is at least as large as another one.
     * @param a The other XBitArray to compare size with.
     */
    void CheckSameSize(XBitArray &a)
    {
        if (m_Size < a.m_Size)
        {
            int newDwords = a.m_Size >> 5;
            int dwords = m_Size >> 5;
            XDWORD *temp = Allocate(newDwords);

            if (temp && m_Data)
            {
                memcpy(temp, m_Data, dwords * sizeof(XDWORD));
                memset(temp + dwords, 0, (newDwords - dwords) * sizeof(XDWORD));
            }

            Free();

            m_Data = temp;
            m_Size = a.m_Size;
        }
    }

    /**
     * @brief Checks if the n-th bit is set to 1.
     * @param n The index of the bit to check.
     * @return A non-zero value if the bit is set, or 0 if it is not set or `n` is out of range.
     */
    int IsSet(int n) const
    {
        if (n >= m_Size)
            return 0;
        return (m_Data[n >> 5] & (1U << (n & 31)));
    }

    /**
     * @brief Provides read-only array-style access to check if a bit is set.
     * @param n The index of the bit to check.
     * @return The result of `IsSet(n)`.
     */
    int operator[](int n) const
    {
        return IsSet(n);
    }

    /**
     * @brief Appends a sequence of bits from an integer to the array.
     * @param n The starting bit index in the array to write to.
     * @param v The integer containing the source bits.
     * @param bitcount The number of bits to append from `v`.
     */
    void AppendBits(int n, int v, int bitcount)
    {
        int mask = 1;
        int end = n + bitcount;
        for (int i = n; i < end; ++i, mask <<= 1)
        {
            if (mask & v)
                Set(i);
            else
                Unset(i);
        }
    }

    /**
     * @brief Sets the n-th bit to 1.
     * @param n The index of the bit to set. The array will be resized if necessary.
     */
    void Set(int n)
    {
        if (n < 0)
            return;
        CheckSize(n);
        m_Data[n >> 5] |= (1U << (n & 31));
    }

    /**
     * @brief Sets the n-th bit to 1 and reports its previous state.
     * @param n The index of the bit to set.
     * @return 1 if the bit was previously 0, or 0 if it was already 1.
     */
    int TestSet(int n)
    {
        if (n < 0)
            return 0;
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

    /**
     * @brief Sets the n-th bit to 0.
     * @param n The index of the bit to unset.
     */
    void Unset(int n)
    {
        if (n < m_Size && n >= 0)
        {
            m_Data[n >> 5] &= ~(1U << (n & 31));
        }
    }

    /**
     * @brief Sets the n-th bit to 0 and reports its previous state.
     * @param n The index of the bit to unset.
     * @return 1 if the bit was previously 1, or 0 if it was already 0.
     */
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
        }
        return 0;
    }

    /**
     * @brief Returns the total number of allocated bits (the capacity).
     */
    int Size() const
    {
        return m_Size;
    }

    /**
     * @brief Resets all bits in the array to 0.
     */
    void Clear()
    {
        if (m_Data)
            memset(m_Data, 0, (m_Size >> 3));
    }

    /**
     * @brief Sets all allocated bits in the array to 1.
     */
    void Fill()
    {
        if (m_Data)
            memset(m_Data, 0xff, (m_Size >> 3));
    }

    /**
     * @brief Performs a bitwise AND operation with another array, storing the result in this array.
     * @param a The other XBitArray.
     */
    void And(XBitArray &a)
    {
        int dwords1 = m_Size >> 5;
        int dwords2 = a.m_Size >> 5;
        int dwords = XMin(dwords1, dwords2);
        int i;
        for (i = 0; i < dwords; ++i)
            m_Data[i] &= a.m_Data[i];
        for (i = dwords; i < dwords1; ++i)
            m_Data[i] = 0;
    }

    /**
     * @brief Performs a bitwise subtraction (`this = this & ~a`).
     * @param a The XBitArray containing bits to clear from this array.
     * @return A reference to this modified array.
     */
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

    /**
     * @brief Checks if there is at least one common set bit between this array and another.
     * @param a The other XBitArray.
     * @return TRUE if `(this & a)` is not zero, FALSE otherwise.
     */
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

    /**
     * @brief Performs a bitwise OR operation with another array, storing the result in this array.
     * @param a The other XBitArray. This array will be resized if `a` is larger.
     */
    void Or(XBitArray &a)
    {
        CheckSameSize(a);
        int size = a.m_Size >> 5;
        for (int i = 0; i < size; ++i)
            m_Data[i] |= a.m_Data[i];
    }

    /**
     * @brief Performs a bitwise XOR operation with another array, storing the result in this array.
     * @param a The other XBitArray. This array will be resized if `a` is larger.
     */
    void XOr(XBitArray &a)
    {
        CheckSameSize(a);
        int size = a.m_Size >> 5;
        for (int i = 0; i < size; ++i)
            m_Data[i] ^= a.m_Data[i];
    }

    /**
     * @brief Inverts all bits in the array.
     */
    void Invert()
    {
        int size = m_Size >> 5;
        for (int i = 0; i < size; ++i)
            m_Data[i] = ~m_Data[i];
    }

    /**
     * @brief Counts the number of set bits (1s) in the array.
     * @return The total number of set bits.
     */
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

    /**
     * @brief Finds the index of the n-th set bit (1).
     * @param n The zero-based index of the set bit to find (e.g., n=0 for the first set bit).
     * @return The bit index of the n-th set bit, or -1 if not found.
     */
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

    /**
     * @brief Finds the index of the n-th unset bit (0).
     * @param n The zero-based index of the unset bit to find.
     * @return The bit index of the n-th unset bit. The array may be resized if not found within the current capacity.
     */
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
        CheckSize(pos);
        return pos;
    }

    /**
     * @brief Converts the bit array to a string representation.
     * @param buffer A character buffer to store the resulting string of '0's and '1's.
     *               It must be large enough to hold `Size()` characters plus a null terminator.
     * @return The provided buffer containing the string representation.
     */
    char *ConvertToString(char *buffer)
    {
        if (buffer)
        {
            for (int i = 0; i < m_Size; i++)
            {
                buffer[i] = (m_Data[i >> 5] & (1U << (i & 31))) ? '1' : '0';
            }
            buffer[m_Size] = '\0';
        }
        return buffer;
    }

    /**
     * @brief Returns the total memory occupied by the allocated buffer in bytes.
     * @param addstatic If TRUE, adds the `sizeof(XBitArray)` to the result.
     * @return The memory size in bytes.
     */
    int GetMemoryOccupation(XBOOL addstatic = FALSE) const
    {
        return (m_Size >> 5) * sizeof(XDWORD) + (addstatic ? sizeof(*this) : 0);
    }

private:
    /**
     * @brief Allocates raw memory for a given number of XDWORDs.
     * @internal
     */
    XDWORD *Allocate(int size)
    {
#ifndef VX_MALLOC
        return new XDWORD[size];
#else
        return (XDWORD *)VxMalloc(sizeof(XDWORD) * size);
#endif
    }

    /**
     * @brief Frees the raw memory block.
     * @internal
     */
    void Free()
    {
        if (m_Data)
        {
#ifndef VX_MALLOC
            delete[] m_Data;
#else
            VxFree(m_Data);
#endif
            m_Data = NULL;
        }
    }

    XDWORD *m_Data; ///< @internal Pointer to the array of 32-bit integers storing the bits.
    int m_Size;     ///< @internal The total allocated size of the array, in bits.
};

#endif // XBITARRAY_H
