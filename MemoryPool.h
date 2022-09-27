#ifndef _MEMORYPOOL_HPP_
#define _MEMORYPOOL_HPP_

#include <iostream>
#include <mutex>

//һ���ڴ�������ڴ��С,������չ
#define MAX_MEMORY_SIZE 256

class MemoryPool;

//�ڴ��
struct MemoryBlock
{
	MemoryBlock* pNext;//��һ���ڴ��
	bool bUsed;//�Ƿ�ʹ��
	bool bBelong;//�Ƿ������ڴ��
	MemoryPool* pMem;//�����ĸ�����
};

class MemoryPool
{
public:
	MemoryPool(size_t nSize=128,size_t nBlock=10)
	{
		//�൱������10���ڴ棬ÿ���ڴ���1024
		_nSize = nSize;
		_nBlock = nBlock;
		_pHeader = NULL;
		_pBuffer = NULL;
	}
	
	virtual ~MemoryPool()
	{
		if (_pBuffer != NULL)
		{
			free(_pBuffer);
		}
	}
	//�����ڴ�
	void* AllocMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//����׵�ַΪ�գ�˵��û������ռ�
		if (_pBuffer == NULL)
		{
			InitMemory();
		}
		MemoryBlock* pRes = NULL;
		//����ڴ�ز�����ʱ����Ҫ���������ڴ�
		if (_pHeader == NULL)
		{
			pRes = (MemoryBlock*)malloc(nSize+sizeof(MemoryBlock));
			pRes->bBelong = false;
			pRes->bUsed = false;
			pRes->pNext = NULL;
			pRes->pMem = NULL;
		}
		else
		{
			pRes = _pHeader;
			_pHeader = _pHeader->pNext;
			pRes->bUsed = true;
		}
		//����ֻ����ͷ�������Ϣ
		return ((char*)pRes + sizeof(MemoryBlock));
	}
	
	//�ͷ��ڴ�
	void FreeMemory(void* p)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//�������ڴ�պ��෴��������Ҫ����ͷ��Ȼ��ȫ���ͷ�
		MemoryBlock* pBlock = ((MemoryBlock*)p - sizeof(MemoryBlock));
		if (pBlock->bBelong)
		{
			pBlock->bUsed = false;
			//ѭ��������
			pBlock->pNext = _pHeader;
			pBlock = _pHeader;
		}
		else
		{
		//�������ڴ��ֱ���ͷžͿ���
			free(pBlock);
		}
	}
	//��ʼ���ڴ��
	void InitMemory()
	{
		if (_pBuffer)
		return;
		//����ÿ��Ĵ�С
		size_t PoolSize = _nSize + sizeof(MemoryBlock);
		//������Ҫ��������ڴ�
		size_t BuffSize = PoolSize * _nBlock;
		_pBuffer = (char*)malloc(BuffSize);
		//��ʼ��ͷ
		_pHeader = (MemoryBlock*)_pBuffer;
		_pHeader->bUsed = false;
		_pHeader->bBelong = true;
		_pHeader->pMem = this;
		//��ʼ��_nBlock�飬�������������ʽ����
		//����ͷָ��
		MemoryBlock* tmp1 = _pHeader;
		for (size_t i = 1; i < _nBlock; i++)
		{
			MemoryBlock* tmp2 = (MemoryBlock*)(_pBuffer + i*PoolSize);
			tmp2->bUsed = false;
			tmp2->pNext = NULL;
			tmp2->bBelong = true;
			_pHeader->pMem = this;
			tmp1->pNext = tmp2;
			tmp1 = tmp2;
		}
	}
public:
	//�ڴ��׵�ַ����һ���ڴ�ĵ�ַ��
	char* _pBuffer;
	//�ڴ��ͷ
	MemoryBlock* _pHeader;
	//�ڴ���С
	size_t _nSize;
	//���ٿ�
	size_t _nBlock;
	
	std::mutex _mutex;
};

//����ʹ��ģ�崫�ݲ���
template<size_t nSize,size_t nBlock>
class MemoryPoolor:public MemoryPool
{
public:
	MemoryPoolor()
	{
		_nSize = nSize;
		_nBlock = nBlock;
	}
};

//��Ҫ���¶��ڴ�ؾ��й���
class ManagerPool
{
public:
	static ManagerPool& Instance()
	{
		static ManagerPool memPool;
		return memPool;
	}
	
	void* AllocMemory(size_t nSize)
	{
		if (nSize < MAX_MEMORY_SIZE)
		{
			return _Alloc[nSize]->AllocMemory(nSize);
		}
		else
		{
			MemoryBlock* pRes = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pRes->bBelong = false;
			pRes->bUsed = true;
			pRes->pMem = NULL;
			pRes->pNext = NULL;
			return ((char*)pRes + sizeof(MemoryBlock));
		}
	}
	
	//�ͷ��ڴ�
	void FreeMemory(void* p)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)p - sizeof(MemoryBlock));
		//�ͷ��ڴ��
		if (pBlock->bBelong)
		{
			pBlock->pMem->FreeMemory(p);
		}
		else
		{
			free(pBlock);
		}
	}

private:
	ManagerPool()
	{
		InitArray(0,128, &_memory128);
		InitArray(129, 256, &_memory256);
	}
	
	~ManagerPool(){}
	
	void InitArray(int nBegin,int nEnd, MemoryPool*pMemPool)
	{
		for (int i = nBegin; i <= nEnd; i++)
		{
			_Alloc[i] = pMemPool;
		}
	}
	//���Ը��ݲ�ͬ�ڴ����з���
	MemoryPoolor<128, 1000> _memory128;
	MemoryPoolor<256, 1000> _memory256;
	//ӳ������
	MemoryPool* _Alloc[MAX_MEMORY_SIZE + 1];
};
#endif
