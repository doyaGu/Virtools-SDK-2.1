// Load the modern standard library before the SDK's legacy nullptr macro.
#include <algorithm>
#include <new>
#include <stdio.h>
#if defined(_MSC_VER) && _MSC_VER >= 1900
#include <initializer_list>
#include <utility>
#endif
#include "XArray.h"
#include "XClassArray.h"
#include "XHashTable.h"
#include "XSHashTable.h"

// Keep checks active in Release builds, too.
#define CHECK(condition) \
    do \
    { \
        if (!(condition)) \
        { \
            fprintf(stderr, "%s:%d: %s failed\n", __FILE__, __LINE__, #condition); \
            return false; \
        } \
    } while (0)

struct ConstantHash
{
    int operator()(int) const { return 0; }
};

struct WraparoundHash
{
    int operator()(int) const { return 15; }
};

bool ArrayOwnElementWhileGrowingTest()
{
    XArray<int> array;
    array.PushBack(7);
    array.PushBack(8);
    CHECK(array.Size() == array.Allocated());
    array.PushBack(array[0]);
    CHECK(array.Size() == 3);
    CHECK(array[2] == 7);

#if VX_HAS_CXX11
    array.EmplaceBack(array[1]);
    CHECK(array.Size() == array.Allocated());
    array.EmplaceBack(array[3]);
    CHECK(array.Size() == 5);
    CHECK(array[3] == 8);
    CHECK(array[4] == 8);

    XArray<int> moved;
    moved.PushBack(11);
    moved.PushBack(12);
    CHECK(moved.Size() == moved.Allocated());
    moved.PushBack(std::move(moved[0]));
    CHECK(moved.Size() == 3);
    CHECK(moved[2] == 11);
#endif
    return true;
}

bool ArrayReserveTest()
{
    XArray<int> array;
    array.Reserve(0);
    array.Fill(9);
    array.Memset(0);
    CHECK(array.Size() == 0);
    CHECK(array.Allocated() == 0);
    array.Reserve(4);
    array.PushBack(7);
    array.PushBack(8);
    array.PushBack(9);
    array.Reserve(8);
    CHECK(array.Size() == 3);
    CHECK(array.Allocated() == 8);
    CHECK(array[0] == 7 && array[1] == 8 && array[2] == 9);
    array.Reserve(2);
    CHECK(array.Size() == 2);
    CHECK(array.Allocated() == 2);
    CHECK(array[0] == 7 && array[1] == 8);
    array.Fill(42);
    CHECK(array[0] == 42 && array[1] == 42);
    array.Memset(0);
    CHECK(array[0] == 0 && array[1] == 0);
#ifdef NDEBUG
    // Invalid capacities assert in Debug and leave the array intact in Release.
    array.Reserve(-1);
    CHECK(array.Size() == 2 && array.Allocated() == 2);
#endif
    array.Reserve(0);
    CHECK(array.Size() == 0 && array.Allocated() == 0);
    array.PushBack(13);
    CHECK(array[0] == 13);

    XClassArray<int> objects;
    objects.PushBack(1);
    objects.PushBack(2);
    objects.PushBack(3);
    CHECK(objects.Size() == 3 && objects[2] == 3);
    return true;
}

template <class H>
bool HashTombstoneProbeChainTest()
{
    XSHashTable<int, int, H> table(16);
    CHECK(table.Insert(1, 10, FALSE));
    CHECK(table.Insert(2, 20, FALSE));
    CHECK(table.Insert(3, 30, FALSE));
    table.Remove(1);
    CHECK(table.FindPtr(2) && *table.FindPtr(2) == 20);
    CHECK(table.FindPtr(3) && *table.FindPtr(3) == 30);
    CHECK(table.IsHere(2) == 1);
    CHECK(table.IsHere(1) == 0);
    CHECK(!table.Insert(2, 200, FALSE));
    CHECK(table.Size() == 2);
    CHECK(table.Insert(2, 200, TRUE));
    CHECK(table.Size() == 2 && *table.FindPtr(2) == 200);
    CHECK(table.Insert(4, 40, FALSE));
    CHECK(table.Size() == 3 && *table.FindPtr(4) == 40);
    table.Remove(4);
    table.Remove(3);
    CHECK(table.Size() == 1 && !table.FindPtr(3));
    CHECK(table.FindPtr(2) && *table.FindPtr(2) == 200);
    table[2] = 201;
    CHECK(table.Size() == 1 && *table.FindPtr(2) == 201);
    table.InsertUnique(2, 202);
    CHECK(table.Size() == 1 && *table.FindPtr(2) == 201);
#if VX_HAS_CXX11
    table.TestInsert(2, 203);
    CHECK(table.Size() == 1 && *table.FindPtr(2) == 201);
    int replacement = 204;
    CHECK(!table.Insert(2, std::move(replacement), FALSE));
    CHECK(table.Insert(2, std::move(replacement), TRUE));
    CHECK(table.Size() == 1 && *table.FindPtr(2) == 204);
    table.EmplaceUnique(2, 205);
    table.TestEmplace(2, 206);
    CHECK(table.Size() == 1 && *table.FindPtr(2) == 204);
#endif
    CHECK(table.Insert(5, 50, FALSE));
    CHECK(table.Size() == 2 && *table.FindPtr(5) == 50);
    return true;
}

bool HashGrowthTest()
{
    XHashTable<int, int, ConstantHash> table(0);
    for (int i = 0; i < 100; ++i)
        CHECK(table.Insert(i, i * 3, FALSE));
    table.Reserve(1000);
    CHECK(table.Size() == 100);
    for (int i = 0; i < 100; ++i)
        CHECK(table.FindPtr(i) && *table.FindPtr(i) == i * 3);
    XHashTable<int, int, ConstantHash> copy(table);
    table.Clear();
    CHECK(table.Insert(1, 10, FALSE));
    CHECK(table.Size() == 1 && *table.FindPtr(1) == 10);
    CHECK(copy.Size() == 100 && *copy.FindPtr(99) == 297);
#if VX_HAS_CXX11
    XHashTable<int, int, ConstantHash> moved(std::move(copy));
    CHECK(moved.Size() == 100 && *moved.FindPtr(99) == 297);
    CHECK(copy.Insert(2, 20, FALSE));
    CHECK(copy.Size() == 1 && *copy.FindPtr(2) == 20);
#endif
    return true;
}

#if VX_HAS_CXX11
bool HashMovedFromReuseTest(bool assignment)
{
    for (int initialCount = 0; initialCount <= 32; initialCount += 32)
    {
        for (int insertionApi = 0; insertionApi < 12; ++insertionApi)
        {
            XHashTable<int, int> source(4);
            for (int i = 0; i < initialCount; ++i)
                source.Insert(i, i * 13);
            XHashTable<int, int> destination(4);
            if (assignment)
            {
                destination.Insert(-1, -13);
                destination = std::move(source);
                CHECK(!destination.FindPtr(-1));
            }
            else
            {
                XHashTable<int, int> moved(std::move(source));
                destination = std::move(moved);
            }

            CHECK(source.Size() == 0 && source.Begin() == source.End());
            const XHashTable<int, int> &empty = source;
            CHECK(empty.Begin() == empty.End());
            CHECK(empty.Find(999) == empty.End());
            CHECK(!empty.FindPtr(999) && !empty.IsHere(999));
            int output = -1;
            CHECK(!empty.LookUp(999, output) && output == -1);
            source.Remove(999);

            const int value = 42;
            switch (insertionApi)
            {
            case 0: CHECK(source.Insert(999, value, FALSE)); break;
            case 1: CHECK(source.Insert(999, 42, FALSE)); break;
            case 2: CHECK(*source.Insert(999, value) == 42); break;
            case 3: CHECK(*source.Insert(999, 42) == 42); break;
            case 4: CHECK(*source.InsertUnique(999, value) == 42); break;
            case 5: CHECK(*source.InsertUnique(999, 42) == 42); break;
            case 6: CHECK(source.TestInsert(999, value).m_New); break;
            case 7: CHECK(source.TestInsert(999, 42).m_New); break;
            case 8: source[999] = 42; break;
            case 9: CHECK(*source.Emplace(999, 42) == 42); break;
            case 10: CHECK(*source.EmplaceUnique(999, 42) == 42); break;
            case 11: CHECK(source.TestEmplace(999, 42).m_New); break;
            }
            CHECK(source.Size() == 1 && source.FindPtr(999));
            CHECK(*source.FindPtr(999) == 42);
            CHECK(destination.Size() == initialCount && !destination.FindPtr(999));
            for (int i = 0; i < initialCount; ++i)
                CHECK(destination.FindPtr(i) && *destination.FindPtr(i) == i * 13);

            for (int i = 0; i < 40; ++i)
                source.Insert(i, i * 7);
            CHECK(source.Size() == 41);
            source.Remove(999);
            XHashTable<int, int> again(std::move(source));
            CHECK(again.Size() == 40);
            for (int i = 0; i < 40; ++i)
                CHECK(again.FindPtr(i) && *again.FindPtr(i) == i * 7);
            source[1000] = 50;
            CHECK(source.FindPtr(1000) && *source.FindPtr(1000) == 50);
            CHECK(!again.FindPtr(1000));
        }
    }
    return true;
}
#endif

int main(int argc, char **argv)
{
    const char *test = argc > 1 ? argv[1] : "all";
    bool passed = true;
    if (!strcmp(test, "all") || !strcmp(test, "array-growth"))
        passed = ArrayOwnElementWhileGrowingTest() && passed;
    if (!strcmp(test, "all") || !strcmp(test, "array-reserve"))
        passed = ArrayReserveTest() && passed;
    if (!strcmp(test, "all") || !strcmp(test, "hash-collision"))
        passed = HashTombstoneProbeChainTest<ConstantHash>() && passed;
    if (!strcmp(test, "all") || !strcmp(test, "hash-wraparound"))
        passed = HashTombstoneProbeChainTest<WraparoundHash>() && passed;
    if (!strcmp(test, "all") || !strcmp(test, "hash-growth"))
        passed = HashGrowthTest() && passed;
#if VX_HAS_CXX11
    if (!strcmp(test, "all") || !strcmp(test, "hash-move-constructor"))
        passed = HashMovedFromReuseTest(false) && passed;
    if (!strcmp(test, "all") || !strcmp(test, "hash-move-assignment"))
        passed = HashMovedFromReuseTest(true) && passed;
#endif
    return passed ? 0 : 1;
}
