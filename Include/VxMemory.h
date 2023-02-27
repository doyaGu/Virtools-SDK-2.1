#ifndef VXMEMORY_H
#define VXMEMORY_H

#include <new>

#include "VxMathDefines.h"

//------ Memory Management

VX_EXPORT void *mynew(unsigned int n);
VX_EXPORT void mydelete(void *a);

VX_EXPORT void *mynewrarray(unsigned int n);
VX_EXPORT void mydeletearray(void *a);

//---- Aligned memory allocation
VX_EXPORT void *VxNewAligned(int size, int align);
VX_EXPORT void VxDeleteAligned(void *ptr);

#ifndef VxMalloc
#define VxMalloc(n) mynew(n)
#endif

#ifndef VxFree
#define VxFree(a) mydelete(a)
#endif

#ifndef VxNew
#define VxNew(x) new (VxMalloc(sizeof(x))) x
#endif

template <class T>
void VxDelete(T *ptr)
{
    if (ptr)
    {
        void *tmp = ptr;
        (ptr)->~T();
        VxFree(tmp);
    }
}

template <class T>
T *VxAllocate(unsigned int cnt)
{
    T *ptr = (T *)VxMalloc(sizeof(T) * cnt);
    if (ptr)
    {
        for (unsigned int i = 0; i < cnt; ++i)
            new (&ptr[i]) T;
    }
    return ptr;
}

template <class T>
void VxDeallocate(T *ptr, unsigned int cnt)
{
    if (ptr)
    {
        void *tmp = ptr;
        for (unsigned int i = 0; i < cnt; ++i)
            (&ptr[i])->~T();
        VxFree(tmp);
    }
}

#ifndef VxNewArray
#define VxNewArray(x, cnt) (x *)VxMalloc(sizeof(x) * cnt)
#endif

#ifndef VxDeleteArray
#define VxDeleteArray(ptr) VxFree(ptr)
#endif

#endif // VXMEMORY_H