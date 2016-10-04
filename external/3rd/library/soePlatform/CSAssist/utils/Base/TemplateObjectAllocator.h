// TemplateCBlockAlloc.h: interface for the CBlockAlloc class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _TEMPLATE_COBJECTALLOCATOR_H
#define _TEMPLATE_COBJECTALLOCATOR_H

#if defined _MT || defined _REENTRANT
#   define USE_ALLOCATOR_MUTEX
#endif


#include <new>
#include "Types.h"
#include "Mutex.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{
    const uint32 MAX_BLOCK_COUNT = 32;

    template <class TYPE>
    class CObjectAllocator  
    {
        public:
            CObjectAllocator(uint32 size=32)
            {
                if (size < 32)
                    size = 32;

                for (uint32 index=0; index<MAX_BLOCK_COUNT; index++)
                    mMemoryBlock[index] = 0;

                mMemoryBlockCount = 0;
                mObjectsAllocated = 0;
                mObjectCount = 0;
                mUnusedList = 0;

                mBytesAllocated = 0;

                Allocate(size);
            }

	        ~CObjectAllocator()
            {
                for (uint32 index=0; index<MAX_BLOCK_COUNT; index++)
                    delete[] mMemoryBlock[index];
            }

            TYPE *Construct()
            {
#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Lock();
#endif
                if (!mUnusedList)
                    Allocate(mObjectCount);
        
                Node *node = mUnusedList;
                mUnusedList = mUnusedList->next;

                new (node) TYPE;
        
                mObjectCount++;
#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Unlock();
#endif
                return (TYPE *)node;
            }

            TYPE *Construct(const TYPE& object)
            {
#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Lock();
#endif
                if (!mUnusedList)
                    Allocate(mObjectCount);
        
                Node *node = mUnusedList;
                mUnusedList = mUnusedList->next;

                new (node) TYPE(object);
        
                mObjectCount++;
#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Unlock();
#endif
                return (TYPE *)node;
            }

            void Destroy(TYPE *object)
            {
                if (object == nullptr)
                    return;

#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Lock();
#endif
                object->~TYPE();

                Node *node = reinterpret_cast<Node *>(object);

                node->next = mUnusedList;
                mUnusedList = node;

                mObjectCount--;
#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Unlock();
#endif
            }

            TYPE *FastConstruct()
            {
                if (!mUnusedList)
                    Allocate(mObjectCount);
        
                Node *node = mUnusedList;
                mUnusedList = mUnusedList->next;

                new (node) TYPE;
        
                mObjectCount++;
                return (TYPE *)node;
            }

            TYPE *FastConstruct(const TYPE& object)
            {
                if (!mUnusedList)
                    Allocate(mObjectCount);
        
                Node *node = mUnusedList;
                mUnusedList = mUnusedList->next;

                new (node) TYPE(object);
        
                mObjectCount++;
                return (TYPE *)node;
            }

            void FastDestroy(TYPE *object)
            {
                if (object == nullptr)
                    return;

                object->~TYPE();

                Node *node = reinterpret_cast<Node *>(object);

                node->next = mUnusedList;
                mUnusedList = node;

                mObjectCount--;
            }

        private:
            struct Node
            {
                char buffer[sizeof(TYPE)];
                Node*  next;
            };

		    bool Allocate(uint32 size)
    		{
                if (mMemoryBlockCount == MAX_BLOCK_COUNT)
                    return false;

                Node* newMemoryBlock = new Node[size];
                mBytesAllocated += size * sizeof(Node);
			    
                for (uint32 i=0; i<size-1; i++)
				    newMemoryBlock[i].next = &newMemoryBlock[i+1];
			    newMemoryBlock[size-1].next = mUnusedList;

			    mUnusedList = newMemoryBlock;
                mMemoryBlock[mMemoryBlockCount++] = newMemoryBlock;
                mObjectsAllocated += size;

                return true;
	    	};
    
            Node*       mMemoryBlock[MAX_BLOCK_COUNT];
            uint32      mMemoryBlockCount;	
            uint32      mObjectsAllocated;
            uint32      mObjectCount;
            Node*       mUnusedList;
#ifdef USE_ALLOCATOR_MUTEX
            CMutex      mMutex;
#endif

            uint64      mBytesAllocated;
    };
};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif  //  _TEMPLATE_CBLOCKALLOC_H
