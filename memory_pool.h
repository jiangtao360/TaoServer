#ifndef _MEMORY_POOL_H
#define _MEMORY_POOL_H

#include "header.h"
#include "utility.h"

class MemoryPool
{
public:
	struct MemPoolItem
	{
		MemPoolItem *next;
		char data[];
	};

	struct MemPoolBlock
	{
		MemPoolBlock *next;
		char items[];
	};
	
	MemoryPool();
	MemoryPool(long item_size, long block_size);
	~MemoryPool();

	void Initialize();
	void* Malloc();
	void Free(void *p);
	void Reconstruct(long item_size);
	void Clean();

private:
	void FreeMemBlock(MemPoolBlock *p);

private:
	long mempool_item_size;
	long mempool_block_size;
	MemPoolItem *pfree_item_header;
	MemPoolBlock *pmem_block_header;
};

#endif
