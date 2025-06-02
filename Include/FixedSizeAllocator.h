#ifndef FIXEDSIZEALLOCATOR_H
#define FIXEDSIZEALLOCATOR_H

/////////////////////////////////////
// XFixedSizeAllocator
// created  : AGoTH
// Date		: 05/12/01
//
// This class is to allocate fixed size object
// with a constant time allocation and deallocation
#include <limits.h>

#include "XArray.h"
#include "XBitArray.h"

class XFixedSizeAllocator
{
public:
    enum { DEFAULT_CHUNK_SIZE = 4096 };

    XFixedSizeAllocator(const int iBlockSize, const int iPageSize = DEFAULT_CHUNK_SIZE)
        : m_PageSize(iPageSize), m_BlockSize(iBlockSize < sizeof(int) ? sizeof(int) : iBlockSize), // Ensure block size is at least sizeof(int) for free list
          m_AChunk(NULL), m_DChunk(NULL)
    {
        // Calculate how many blocks fit in a page
        m_BlockCount = m_PageSize / m_BlockSize;
        if (m_BlockCount == 0)
            m_BlockCount = 1; // At least one block per chunk
    }

    ~XFixedSizeAllocator()
    {
        Clear();
    }

    // return the number of allocated chunks
    int GetChunksCount()
    {
        return m_Chunks.Size();
    }

    // return the total size of all chunks
    int GetChunksTotalSize()
    {
        return m_Chunks.Size() * m_PageSize;
    }

    // return the occupied memory in chunks
    int GetChunksOccupation()
    {
        int occupation = 0;
        for (Chunks::Iterator it = m_Chunks.Begin(); it != m_Chunks.End(); ++it)
        {
            occupation += (it->m_BlockCount - it->m_BlockAvailables) * m_BlockSize;
        }
        return occupation;
    }

    template <class T>
    void CallDtor(T *iDummy)
    {
        // we clear all the chunks
        for (Chunks::Iterator it = m_Chunks.Begin(); it != m_Chunks.End(); ++it)
        {
            it->CallDtor(iDummy, m_BlockSize, m_BlockCount);
        }
    }

    void Clear()
    {
        // Destroy all chunks
        for (Chunks::Iterator it = m_Chunks.Begin(); it != m_Chunks.End(); ++it)
        {
            it->Destroy();
        }
        m_Chunks.Clear();
        m_AChunk = NULL;
        m_DChunk = NULL;
    }

    void *Allocate()
    {
        // Try to allocate from current allocating chunk
        if (m_AChunk && m_AChunk->m_BlockAvailables > 0)
        {
            return m_AChunk->Allocate(m_BlockSize);
        }

        // Find a chunk with available blocks
        for (Chunks::Iterator it = m_Chunks.Begin(); it != m_Chunks.End(); ++it)
        {
            if (it->m_BlockAvailables > 0)
            {
                m_AChunk = &(*it);
                return m_AChunk->Allocate(m_BlockSize);
            }
        }

        // No chunks with available blocks, create a new one
        Chunk newChunk;
        newChunk.Init(m_BlockSize, m_BlockCount);
        m_Chunks.PushBack(newChunk);
        m_AChunk = &m_Chunks.Back();
        return m_AChunk->Allocate(m_BlockSize);
    }

    void Free(void *iP)
    {
        if (!iP)
            return;

        // Find the chunk containing this pointer
        Chunk *chunk = FindChunk(iP);
        if (chunk)
        {
            chunk->Deallocate(iP, m_BlockSize);
            m_DChunk = chunk; // Cache for next deallocation
        }
    }

private:
    class Chunk
    {
    public:
        Chunk() : m_Data(NULL), m_FirstAvailableBlock(0), m_BlockAvailables(0), m_BlockCount(0) {}

        void Init(size_t iBlockSize, unsigned int iBlockCount)
        {
            m_BlockCount = iBlockCount;
            m_BlockAvailables = iBlockCount;
            m_FirstAvailableBlock = 0;

            // Allocate memory for the chunk
            m_Data = new unsigned char[iBlockCount * iBlockSize];

            // Initialize free list - each free block contains index of next free block
            for (unsigned int i = 0; i < iBlockCount - 1; ++i)
            {
                *(unsigned int *)(m_Data + i * iBlockSize) = i + 1;
            }
            // Last block marks end of free list
            *(unsigned int *)(m_Data + (iBlockCount - 1) * iBlockSize) = UINT_MAX;
        }

        template <class T>
        void CallDtor(T *iDummy, size_t iBlockSize, unsigned int iBlockCount)
        {
            // everything is clear -> nothing to do
            if (m_BlockAvailables == iBlockCount)
                return;

            // else we have some cleaning todo

            if (!m_BlockAvailables)
            {
                // we need to clean everything
                // we call the dtor of the used blocks
                for (unsigned int i = 0; i < iBlockCount; ++i)
                {

                    unsigned char *p = m_Data + i * iBlockSize;
                    ((T *)p)->~T();
                }
            }
            else
            {
                // only some are used
                XBitArray freeBlocks;

                {
                    // we mark the objects used
                    int freeb = m_FirstAvailableBlock;
                    for (unsigned int i = 0; i < m_BlockAvailables - 1; ++i)
                    {

                        freeBlocks.Set(freeb);

                        unsigned char *p = m_Data + freeb * iBlockSize;
                        freeb = *(int *)p;
                    }
                    freeBlocks.Set(freeb);
                }

                {
                    // we call the dtor of the used blocks
                    for (unsigned int i = 0; i < iBlockCount; ++i)
                    {

                        if (freeBlocks.IsSet(i))
                            continue;

                        unsigned char *p = m_Data + i * iBlockSize;
                        ((T *)p)->~T();
                    }
                }
            }
        }

        void Destroy()
        {
            delete[] m_Data;
            m_Data = NULL;
            m_BlockCount = 0;
            m_BlockAvailables = 0;
            m_FirstAvailableBlock = 0;
        }

        void *Allocate(size_t iBlockSize)
        {
            if (m_BlockAvailables == 0)
            {
                return NULL; // No available blocks
            }

            // Get the first available block
            unsigned char *result = m_Data + m_FirstAvailableBlock * iBlockSize;

            // Update the free list head to next available block
            m_FirstAvailableBlock = *(unsigned int *)result;
            m_BlockAvailables--;

            return result;
        }

        void Deallocate(void *iP, size_t iBlockSize)
        {
            if (!iP)
                return;

            // Calculate block index within this chunk
            unsigned char *ptr = (unsigned char *)iP;
            unsigned int blockIndex = (unsigned int)((ptr - m_Data) / iBlockSize);

            // Validate the pointer is properly aligned
            if ((ptr - m_Data) % iBlockSize != 0)
            {
                return; // Invalid pointer
            }

            // Add this block to the front of the free list
            *(unsigned int *)iP = m_FirstAvailableBlock;
            m_FirstAvailableBlock = blockIndex;
            m_BlockAvailables++;
        }

        unsigned char *m_Data;
        unsigned int m_FirstAvailableBlock;
        unsigned int m_BlockAvailables;
        unsigned int m_BlockCount;
    };

    // types
    typedef XArray<Chunk> Chunks;

    // function to find the chunk containing the ptr
    Chunk *FindChunk(void *iP)
    {
        // Check cached deallocating chunk first for performance
        if (m_DChunk)
        {
            unsigned char *ptr = (unsigned char *)iP;
            unsigned char *chunkStart = m_DChunk->m_Data;
            unsigned char *chunkEnd = chunkStart + (m_DChunk->m_BlockCount * m_BlockSize);
            if (ptr >= chunkStart && ptr < chunkEnd)
            {
                return m_DChunk;
            }
        }

        // Search all chunks
        for (Chunks::Iterator it = m_Chunks.Begin(); it != m_Chunks.End(); ++it)
        {
            unsigned char *ptr = (unsigned char *)iP;
            unsigned char *chunkStart = it->m_Data;
            unsigned char *chunkEnd = chunkStart + (it->m_BlockCount * m_BlockSize);
            if (ptr >= chunkStart && ptr < chunkEnd)
            {
                return &(*it);
            }
        }

        return NULL; // Not found
    }

    // members
    size_t m_PageSize;
    // Block size
    size_t m_BlockSize;
    // Blocks Count (per Chunk)
    unsigned int m_BlockCount;

    // the chunks
    Chunks m_Chunks;

    // Allocating and deallocating chunks
    Chunk *m_AChunk;
    Chunk *m_DChunk;
};

//
template <class T>
class XObjectPool
{
public:
    XObjectPool(XBOOL iCallDtor = TRUE) : m_Allocator(sizeof(T)), m_CallDtor(iCallDtor) {}

    T *Allocate()
    {
        return new (m_Allocator.Allocate()) T;
    }

    void Free(T *iP)
    {
        if (m_CallDtor)
            iP->~T();

        m_Allocator.Free(iP);
    }

    void Clear()
    {
        if (m_CallDtor)
            m_Allocator.CallDtor((T *)NULL);

        m_Allocator.Clear();
    }

private:
    XFixedSizeAllocator m_Allocator;
    XBOOL m_CallDtor;
};

#endif // FIXEDSIZEALLOCATOR_H