#ifndef VXSHAREDLIBRARY_H
#define VXSHAREDLIBRARY_H

#include "VxMathDefines.h"

/***********************************************************************
Summary: Utility class for loading Dlls.

Remarks:

Example:
        // To load a dll in order to retrieve a specific function and call it
        VxSharedLibrary Shl;
        INSTANCE_HANDLE DllInst = Shl.Load(DllName);	// INSTANCE_HANDLE = HMODULE of win32

        MyFunc = Shl.GetFunctionPtr("MyFunctionInTheDll");
        MyFunc(...);

        shl.ReleaseLibrary();



See also:
***********************************************************************/
class VxSharedLibrary
{
public:
    // Creates an unattached VxLibrary
    VxSharedLibrary() : m_LibraryHandle(NULL) {}

    // Attaches an existing Library to a VxLibrary
    void Attach(INSTANCE_HANDLE LibraryHandle)
    {
        m_LibraryHandle = LibraryHandle;
    }

    // Loads the shared Library from disk
    VX_EXPORT INSTANCE_HANDLE Load(char *LibraryName);

    // Unloads the shared Library
    VX_EXPORT void ReleaseLibrary();

    template <typename T>
    T GetFunction(const char *func)
    {
        return reinterpret_cast<T>(GetFunctionPtr(func));
    }

    // Retrieves a function pointer from the library
    VX_EXPORT void *GetFunctionPtr(char *FunctionName);

protected:
    INSTANCE_HANDLE m_LibraryHandle;
};

#endif // VXSHAREDLIBRARY_H
