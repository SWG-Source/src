#ifndef UTIL_H
#define UTIL_H

#include <list>
#include <vector>

template<typename T>
class ref_ptr
{
private:
    struct Node
    {
        T object;
        unsigned refCount;
    };
    struct Allocator
    {
    public:
        static std::vector<void *> & getBlockArray()
        {
            static std::vector<void *> * blockArray = new std::vector<void *>;
            return *blockArray;
        }
        static std::list<T *> & getBlockList()
        {
            static std::list<T *> * blockList = new std::list<T *>;
            return *blockList;
        }
        static T * allocate() 
        { 
            if (getBlockList().empty())
                grow();
            T * ptr = getBlockList().front(); 
            getBlockList().pop_front();
            _blocksAvailable--;
            return new (ptr) T; 
        }
        static T * allocate(const T & copy) 
        { 
            if (getBlockList().empty())
                grow();
            T * ptr = getBlockList().front(); 
            getBlockList().pop_front();
            _blocksAvailable--;
            return new (ptr) T(copy); 
        }
        static void release(T * ptr) 
        { 
            ptr->~T();
            getBlockList().push_front(ptr); 
            _blocksAvailable++;
            if (_blocksAvailable == _blocksTotal)
            {
                for (unsigned i=0; i<getBlockArray().size(); i++)
                    delete [] static_cast<unsigned *>(getBlockArray()[i]);
                getBlockArray().clear();
                getBlockList().clear();
                _blocksAvailable = _blocksTotal = 0;
            }
        }
    private:
        static void grow() 
        { 
            unsigned blockCount = (unsigned)(getBlockArray().size()+1)*32; 
            unsigned blockSize = (unsigned)blockCount * sizeof(Node)/sizeof(unsigned);
            getBlockArray().push_back(new unsigned[blockSize]); 

            Node * ptr = static_cast<Node *>(getBlockArray().back());
            for (size_t i=0; i<blockCount; i++) 
                getBlockList().push_back(&ptr[i].object); 
            _blocksAvailable += blockCount;
            _blocksTotal += blockCount;
        }
    private:
        static unsigned _blocksAvailable;
        static unsigned _blocksTotal;
    };
public:
    ref_ptr() : ptr(0) {}
    //explicit ref_ptr(T * p) : ptr(p) { addRef(); }
    ref_ptr(const ref_ptr<T> & copy) { copy.checkRef(); ptr = copy.ptr; addRef(); }
    ~ref_ptr() { unref(); }

    //ref_ptr<T> & operator=(T * p) { unref(); ptr = p; addRef(); return *this; }
    ref_ptr<T> & operator=(const ref_ptr<T> & rhs) { rhs.checkRef(); if (&rhs != this && rhs.ptr != ptr) { unref(); ptr = rhs.ptr; addRef(); } return *this; }
    T & operator*() const { checkRef(); return *ptr; }
    T * operator->() const { checkRef(); return ptr; }
    operator bool() const { return ptr != 0; }
    bool unique() const { return (!ptr || node()->refCount == 1); }
    void clone() { if (!unique()) { T * temp = Allocator::allocate(*ptr); unref(); ptr = temp; node()->refCount = 1; } }
private:
    Node * node() const { return static_cast<Node *>((void *)ptr); }
    void checkRef() const { if (!ptr) { ptr = Allocator::allocate(); node()->refCount = 1; } }
    void addRef() const { if (ptr) { node()->refCount++; } }
    void unref() { if (ptr && !--(node()->refCount)) Allocator::release(ptr); }
private:
    mutable T * ptr;
};

template<typename T> 
unsigned ref_ptr<T>::Allocator::_blocksAvailable = 0;
template<typename T> 
unsigned ref_ptr<T>::Allocator::_blocksTotal = 0;


#endif
