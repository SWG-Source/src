#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H

#if defined _MT || defined _REENTRANT
#   define USE_ALLOCATOR_MUTEX
#endif


#include <new>
#include "Mutex.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{
    template<int BLOCK_SIZE>
    class CBlockAllocator  
    {

        private:
            class Node
            {
                public:
                    Node *   mNext;
                    unsigned mBuffer[(BLOCK_SIZE-1)/sizeof(unsigned)+1];
            };
        
        public:
            CBlockAllocator() :
                mMemoryBlockCount(0),
                mNodesAllocated(0),
                mNodesUsed(0),
                mUnusedList(0)
            {
                for (int block=0; block<MAX_BLOCK_COUNT; block++)
                    mMemoryBlock[block] = 0;
                Allocate();
            }

            ~CBlockAllocator()
            {
                for (int block=0; block<MAX_BLOCK_COUNT; block++)
                    delete[] mMemoryBlock[block];
            }

            void * Allocate()
            {
#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Lock();
#endif
            void * data;

            Node * node = mUnusedList;
            mUnusedList = node->mNext;
            node->mNext = (Node *)1;

            data = node->mBuffer;

            mNodesUsed++;

#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Unlock();
#endif

            return data;
            }

            void Deallocate(void * data)
            {
                if (!data)
                    return;

                char * memoryPtr = reinterpret_cast<char *>(data) - sizeof(Node *);
                Node * node = reinterpret_cast<Node *>(memoryPtr);
            
#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Lock();
#endif

                node->mNext = mUnusedList;
                mUnusedList = node;
                mNodesUsed--;

#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Unlock();
#endif
            }

            template<class T> T * Construct(const T & object)
            {
                T * objectPtr;

#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Lock();
#endif
                if (sizeof(T) > BLOCK_SIZE || (!mUnusedList && !Allocate()))
                {
                    char * memoryPtr = reinterpret_cast<char *>(new unsigned[(sizeof(Node *)+sizeof(T)-1)/sizeof(unsigned)+1]);
                    objectPtr = reinterpret_cast<T *>(memoryPtr+sizeof(Node *));

                    *reinterpret_cast<unsigned *>(memoryPtr) = 0;
                }
                else
                {
                    Node * node = mUnusedList;
                    mUnusedList = node->mNext;
                    node->mNext = (Node *)1;

                    objectPtr = reinterpret_cast<T *>(node->mBuffer);

                    mNodesUsed++;
                }

                new (objectPtr) T(object);

#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Unlock();
#endif
                return objectPtr;
            }

            template<class T> void Destroy(T * object)
            {
                if (!object)
                    return;

                char * memoryPtr = reinterpret_cast<char *>(object) - sizeof(Node *);
                Node * node = reinterpret_cast<Node *>(memoryPtr);
            
                object->~T();

#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Lock();
#endif
                if (node->mNext == 0)
                    delete []memoryPtr;
                else
                {
                    node->mNext = mUnusedList;
                    mUnusedList = node;
                    mNodesUsed--;
                }
#ifdef USE_ALLOCATOR_MUTEX
                mMutex.Unlock();
#endif
            }

            template<class T> T * FastConstruct(const T & object)
            {
                T * objectPtr;

                if (sizeof(T) > BLOCK_SIZE || (!mUnusedList && !Allocate()))
                {
                    char * memoryPtr = reinterpret_cast<char *>(new unsigned[(sizeof(Node *)+sizeof(T)-1)/sizeof(unsigned)+1]);
                    objectPtr = reinterpret_cast<T *>(memoryPtr+sizeof(Node *));

                    *reinterpret_cast<unsigned *>(memoryPtr) = 0;
                }
                else
                {
                    Node * node = mUnusedList;
                    mUnusedList = node->mNext;
                    node->mNext = (Node *)1;

                    objectPtr = reinterpret_cast<T *>(node->mBuffer);

                    mNodesUsed++;
                }

                new (objectPtr) T(object);

                return objectPtr;
            }

            template<class T> void FastDestroy(T * object)
            {
                if (!object)
                    return;

                char * memoryPtr = reinterpret_cast<char *>(object) - sizeof(Node *);
                Node * node = reinterpret_cast<Node *>(memoryPtr);
            
                object->~T();

                if (node->mNext == 0)
                    delete []memoryPtr;
                else
                {
                    node->mNext = mUnusedList;
                    mUnusedList = node;
                    mNodesUsed--;
                }
            }

		    bool Allocate()
    	    {
                if (mMemoryBlockCount == MAX_BLOCK_COUNT)
                    return false;

                unsigned count = (mNodesAllocated ? mNodesAllocated : 32);

                Node* newMemoryBlock = new Node[count];
                for (unsigned i=0; i<count-1; i++)
				    newMemoryBlock[i].mNext = &newMemoryBlock[i+1];
			    newMemoryBlock[count-1].mNext = mUnusedList;
            
                mUnusedList = newMemoryBlock;
                mMemoryBlock[mMemoryBlockCount++] = newMemoryBlock;
                mNodesAllocated += count;

                return true;
	        };

        private:
            enum { MAX_BLOCK_COUNT = 32 };

            void *           mMemoryBlock[MAX_BLOCK_COUNT];
            unsigned         mMemoryBlockCount;
            unsigned         mNodesAllocated;
            unsigned         mNodesUsed;
            Node *           mUnusedList;

#ifdef USE_ALLOCATOR_MUTEX
            Base::CMutex     mMutex;
#endif
    };


};

#ifdef EXTERNAL_DISTRO
};
#endif

#endif  //  BLOCK_ALLOCATOR_H
