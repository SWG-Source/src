#include "TcpBlockAllocator.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

TcpBlockAllocator::TcpBlockAllocator(const unsigned initSize, const unsigned initCount)
: m_freeHead(nullptr), m_blockCount(initCount), m_blockSize(initSize), m_numAvailBlocks(0)
{
    realloc();
}

TcpBlockAllocator::~TcpBlockAllocator()
{
    while(m_freeHead)
    {
        data_block *tmp = m_freeHead;
        m_freeHead = m_freeHead->m_next;
        delete[] tmp->m_data;
        delete tmp;m_numAvailBlocks--;
    }
}

data_block *TcpBlockAllocator::getBlock()
{
    data_block *tmp;

    if(!m_freeHead)
    {
        realloc();
    }

    tmp = m_freeHead;
    m_freeHead = m_freeHead->m_next;
    tmp->m_next = nullptr;
    m_numAvailBlocks--;
    return(tmp);
}

void TcpBlockAllocator::returnBlock(data_block *b)
{
    b->m_usedSize = 0;
    b->m_sentSize = 0;

    if (m_numAvailBlocks >= m_blockCount)
    {
        delete[] b->m_data;
        delete b;
        return;
    }
        
    b->m_next = m_freeHead;
    m_freeHead = b; m_numAvailBlocks++;
}

void TcpBlockAllocator::realloc()
{
    data_block *tmp = nullptr, *cursor = nullptr;
    
    tmp = new data_block; m_numAvailBlocks++;
    cursor = tmp;
    memset(cursor, 0, sizeof(data_block));
    cursor->m_data = new char[m_blockSize];
    cursor->m_totalSize = m_blockSize;
    
    for(unsigned i = 1; i < m_blockCount; i++)
    {
        cursor->m_next = new data_block; m_numAvailBlocks++;
        cursor = cursor->m_next;
        memset(cursor, 0, sizeof(data_block));
        cursor->m_data = new char[m_blockSize];
        cursor->m_totalSize = m_blockSize;
    }

    if(m_freeHead)
    {
        cursor->m_next = m_freeHead;
        m_freeHead = tmp;
    }
    else
    {
        m_freeHead = tmp;
    }
}


#ifdef EXTERNAL_DISTRO
};
#endif




