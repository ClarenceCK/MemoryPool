#ifndef _OPERATEMEM_H_
#define _OPERATEMEM_H_

#include <iostream>
#include <stdlib.h>
#include "MemoryPool.h"

void* operator new(size_t nSize)
{
    return ManagerPool::Instance().AllocMemory(nSize);
}

void operator delete(void* p)
{
    return ManagerPool::Instance().FreeMemory(p);
}

void* operator new[](size_t nSize)
{
    return ManagerPool::Instance().AllocMemory(nSize);
}

void operator delete[](void* p)
{
    return ManagerPool::Instance().FreeMemory(p);
}

#endif
