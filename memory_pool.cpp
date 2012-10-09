#include "memory_pool.h"

MemoryPool::MemoryPool() : pfree_item_header(NULL), pmem_block_header(NULL)
{
	
}

MemoryPool:: MemoryPool(long item_size, long block_size) : 
	mempool_item_size(item_size), mempool_block_size(block_size), 
	pfree_item_header(NULL), pmem_block_header(NULL)
{

}

MemoryPool::~MemoryPool()
{
	Clean();
}

void MemoryPool::Initialize()
{
	char buffer[256];

	if(get_profile_string("memory_pool", "mempool_item_size", NULL, buffer, 
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, "%s", 
			"Get mempool_item_size failed when initia memory pool");
	}
	mempool_item_size = atol(buffer);
	mempool_item_size += sizeof(int*)-sizeof(int);

	if(get_profile_string("memory_pool", "mempool_block_size", NULL, buffer, 
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, "%s", 
			"Get mempool_block_size failed when initia memory pool");
	}
	mempool_block_size = atol(buffer); 
	
}

void* MemoryPool::Malloc()
{
	if(pfree_item_header == NULL)
	{
		int count = mempool_block_size/mempool_item_size;
		
		MemPoolBlock *pnew_block = NULL;
		try
		{
			pnew_block = reinterpret_cast<MemPoolBlock*> 
				(new char[mempool_block_size+sizeof(MemPoolBlock*)]);
		}
		catch(const std::bad_alloc &ex)
		{
			global_logger.WriteLog(Logger::ERROR, -1, "Memory pool allocated memory failed");
		}
			
		MemPoolItem *pitem1 = NULL, *pitem2 = NULL;
		for(int i=1; i<count; i++)
		{
			pitem1 = reinterpret_cast<MemPoolItem*>(pnew_block->items+(i-1)*mempool_item_size);
			pitem2 = reinterpret_cast<MemPoolItem*>(pnew_block->items+i*mempool_item_size);
			pitem1->next = pitem2;
		}
		pitem2->next = NULL;
		pfree_item_header = reinterpret_cast<MemPoolItem*> (pnew_block->items);
		pnew_block->next = pmem_block_header;
		pmem_block_header = pnew_block;
	}	
	
	MemPoolItem *pitem = pfree_item_header;
	pfree_item_header = pfree_item_header->next;

	return pitem->data;
}

void MemoryPool::Free(void *p)
{
	MemPoolItem *pitem = reinterpret_cast<MemPoolItem*> 
		(static_cast<char*> (p) - sizeof(MemPoolItem*));
	pitem->next = pfree_item_header;
	pfree_item_header = pitem;
}

void MemoryPool::Reconstruct(long item_size)
{
	MemPoolBlock *pblock = pmem_block_header;
	mempool_item_size = item_size;

	int count = mempool_block_size/mempool_item_size;
	while(pblock)
	{	
		MemPoolItem *pitem1 = NULL, *pitem2 = NULL;
		for(int i=1; i<count; i++)
		{
			pitem1 = reinterpret_cast<MemPoolItem*> (pblock->items+(i-1)*mempool_item_size);
			pitem2 = reinterpret_cast<MemPoolItem*> (pblock->items+i*mempool_item_size);
			pitem1->next = pitem2;
		}
		if(pblock->next != NULL)
			pitem2->next = reinterpret_cast<MemPoolItem*> (pblock->next->items);
		else
			pitem2->next = NULL;
		pblock = pblock->next;
	}

	if(pmem_block_header != NULL)
		pfree_item_header = reinterpret_cast<MemPoolItem*> (pmem_block_header->items);
}

void MemoryPool::Clean()
{
	if(pmem_block_header != NULL)
		FreeMemBlock(pmem_block_header);
	pmem_block_header = NULL;
	pfree_item_header = NULL;
}

void MemoryPool::FreeMemBlock(MemPoolBlock * p)
{
	if(p->next == NULL)
	{
		delete [] reinterpret_cast<char*> (p);
		return;
	}
	FreeMemBlock(p->next);
	delete [] reinterpret_cast<char*> (p);
}

