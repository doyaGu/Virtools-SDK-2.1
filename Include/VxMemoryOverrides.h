#ifndef VXMEMORYOVERRIDES_H
#define VXMEMORYOVERRIDES_H

// This header should be included in only one source file!

#include "VxMemory.h"

void operator delete(void *p) VX_NOEXCEPT { VxFree(p); };
void operator delete[](void *p) VX_NOEXCEPT { VxFree(p); };

void operator delete(void *p, const std::nothrow_t &) VX_NOEXCEPT { VxFree(p); }
void operator delete[](void *p, const std::nothrow_t &) VX_NOEXCEPT { VxFree(p); }

void *operator new(std::size_t n) { return VxMalloc(n); }
void *operator new[](std::size_t n) { return VxMalloc(n); }

void *operator new(std::size_t n, const std::nothrow_t &tag) VX_NOEXCEPT { (void)(tag); return VxMalloc(n); }
void *operator new[](std::size_t n, const std::nothrow_t &tag) VX_NOEXCEPT { (void)(tag); return VxMalloc(n); }

#endif // VXMEMORYOVERRIDES_H