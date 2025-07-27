#ifndef CKMEMORYPOOL_H
#define CKMEMORYPOOL_H

#include "CKContext.h"
#include "VxMemoryPool.h"

class CKMemoryPool
{
public:
    CKMemoryPool(CKContext *context, int size = 0);
    ~CKMemoryPool();

    void *Mem() const;

protected:
    CKContext *m_Context;
    void *m_Memory;
    int m_Index;
};

#endif // CKMEMORYPOOL_H