#ifndef TCPBLOCKALLOCATOR_H
#define TCPBLOCKALLOCATOR_H

#include <string.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

struct data_block
{
    unsigned m_usedSize;
    unsigned m_sentSize;
    unsigned m_totalSize;
    char *m_data; 
    data_block *m_next;
};

class TcpBlockAllocator
{
public:
    TcpBlockAllocator(const unsigned initSize, const unsigned initCount);
    ~TcpBlockAllocator();
    data_block *getBlock();
    void returnBlock(data_block *);

private:
    void realloc();
    data_block *m_freeHead;
    unsigned m_blockCount;
    unsigned m_blockSize;
    unsigned m_numAvailBlocks;
};
 

#ifdef EXTERNAL_DISTRO
};
#endif
#endif //TCPBLOCKALLOCATOR_H




