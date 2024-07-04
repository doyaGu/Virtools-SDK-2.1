#ifndef VXMEMORYPOOL_H
#define VXMEMORYPOOL_H

#include "XUtil.h"

/****************************************************************
Summary: Memory buffer class.

Remarks:
    o This class is designed to be used as a local or global variable that
    handles a memory buffer that will be automatically deleted on destruction.
    o The memory buffer is aligned on a 16 bytes boundary (64 on a PS2 system)
    o Do not call delete on the buffer returned by Buffer() method.
See Also: VxNewAligned,VxDeleteAligned
****************************************************************/
class VxMemoryPool
{
public:
    ~VxMemoryPool()
    {
        VxDeleteAligned(m_Memory);
        m_Memory = NULL;
    }

    // Constructs the class allocating the buffer to size dwords.
    explicit VxMemoryPool(int size = 0)
    {
        m_Memory = NULL;
        m_Allocated = 0;
        Allocate(size);
    }

    // Returns access to the memory buffer.
    void *Buffer() const
    {
        return m_Memory;
    }

    // Returns allocated size of this memory pool.
    int AllocatedSize() const
    {
        return m_Allocated;
    }

    // Allocates the number of dwords if not yet allocated.
    void Allocate(int size)
    {
        if (m_Allocated < size)
        {
            VxDeleteAligned(m_Memory);
            m_Memory = (XBYTE *)VxNewAligned(size * sizeof(XDWORD), 16);
            m_Allocated = size;
        }
    }

protected:
    XBYTE *m_Memory;
    int m_Allocated;
};

#endif // VXMEMORYPOOL_H