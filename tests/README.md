# Container regression tests

These tests cover container fixes ported from Ballanced's VxMath submodule:

- `f7b797e`: preserve arguments referring to an XArray element during PushBack
  and EmplaceBack growth.
- `119d898`: probe past XSHashTable tombstones, find existing keys before
  reusing deleted buckets, and preserve collision chains across wraparound.
- `3b00a6b`: handle XArray reserve sizes safely and copy/fill memory with the
  corrected bounds and byte counts.
- `8b597d2`: initialize XHashTable buckets with Memset and enforce a minimum
  rehash size of four buckets.
- `1706e0e`: return a boolean from XSHashTable::IsHere without narrowing a pointer.
- `61ef70b`: explicitly cast the XClassArray rvalue insertion capacity to int.

The follow-up XHashTable fix is applied in both repositories. Bucket reads now
return no entry when a moved-from table has no buckets, so its first insertion
rebuilds storage through the existing rehash path. Move operations keep their
noexcept contract and do not allocate to reset the source object.

The SDK's existing Attach/Detach API, object layouts, and compiler compatibility
guards are preserved. Tests use the default new[]/delete[] allocation path and
do not need the Virtools runtime or import libraries.

```powershell
cmake -S . -B build/container-port -G "Visual Studio 17 2022" -A Win32 -DVIRTOOLS_SDK_BUILD_TESTS=ON
cmake --build build/container-port --config Release
ctest --test-dir build/container-port -C Release --output-on-failure
```

The same commands accept Debug. There are five cases per language branch plus
two modern cases for reusing the source after move construction and assignment.
Each move case covers twelve insertion APIs with empty and populated source
tables, verifies independent destination contents, and exercises queries,
removal, further growth, and a second move. No Reserve call is required.
The legacy target sets VX_HAS_CXX11=0; with modern MSVC this exercises the SDK's
legacy implementations, but does not establish compatibility with an actual
Visual C++ 6.0 compiler.

For AddressSanitizer, configure a separate build with
`-DCMAKE_CXX_FLAGS="/fsanitize=address /Zi"` and
`-DCMAKE_EXE_LINKER_FLAGS="/INCREMENTAL:NO /DEBUG"`, then build and test Release.
The MSVC x86 compiler directory must be on PATH for its ASan runtime DLL.

Validation: all twelve cases passed in Win32 Debug, Release, and Release with
ASan. Before the move reuse fix, the new move tests and direct reinsertion in
the growth test failed under ASan on a bucket read before rehashing.

During the initial port, the original ten-case suite failed eight cases against
the SDK's original HEAD headers, including heap-use-after-free in XArray and
missing colliding keys after XSHashTable deletion.
