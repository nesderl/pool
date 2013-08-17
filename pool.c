/*
 * pool.c
 *
 *  Created on: 4 janv. 2013
 *      Author: Matthieu
 */


#include <stdlib.h>
#include <stdio.h>

#include "../headers/pool.h"

//----------------------------------------------------------
// Structures
//----------------------------------------------------------
struct Node
{
	struct Node* next;
};

struct Chunk
{
	struct Chunk* next;
	struct Node freeStack;
};

//----------------------------------------------------------
// Prototypes
//----------------------------------------------------------

// Node
static void pushNode(struct Chunk* chunk, struct Node* node);
static struct Node* popNode(struct Chunk* chunk);

// Chunk
static struct Chunk* createChunk(struct Pool* pool);
static void pushChunk(struct Pool* pool, struct Chunk* chunk);
static struct Chunk* findFreeChunk(struct Chunk* chunks);
static struct Chunk* findOwner(struct Pool* pool, void* ptr);

//----------------------------------------------------------
// Node
//----------------------------------------------------------
static void pushNode(struct Chunk* chunk, struct Node* node)
{
	node->next = chunk->freeStack.next;
	chunk->freeStack.next = node;
}

static struct Node* popNode(struct Chunk* chunk)
{
	struct Node* node = chunk->freeStack.next;
	if ( node != NULL)
	{
		chunk->freeStack.next = node->next;
	}

	return (node);
}

//----------------------------------------------------------
// Chunk
//----------------------------------------------------------

static void pushChunk(struct Pool* pool, struct Chunk* chunk)
{
	if ( chunk != NULL && pool != NULL)
	{
		chunk->next = pool->chunkList;
		pool->chunkList = chunk;
	}
}

static struct Chunk *createChunk(struct Pool* pool)
{
	size_t blockSize = 0;
	blockSize += sizeof(struct Chunk);
	blockSize += pool->elementsPerChunk * pool->elementSize;

	void* block = malloc(blockSize);
	if ( block != NULL)
	{
		struct Chunk* chunk = block;
		chunk->freeStack.next = NULL;

		// build the stack
		void* min = chunk+1;

		int i;
		for ( i=0; i < pool->elementsPerChunk; i++)
		{
			pushNode(chunk, min + i*pool->elementSize);
		}

		((struct Node*)(min))->next = NULL;
	}

	return (block);
}

static struct Chunk* findFreeChunk(struct Chunk* chunks)
{
	struct Chunk* result = NULL;

	struct Chunk* it;
	for ( it = chunks; it != NULL; it = it->next)
	{
		if (it->freeStack.next != NULL)
		{
			result = it;
			break;
		}
	}

	return (result);
}

static struct Chunk* findOwner(struct Pool* pool, void* ptr)
{
	size_t blockSize = 0;
	blockSize += sizeof(struct Chunk);
	blockSize += pool->elementsPerChunk * pool->elementSize;

	struct Chunk* result = NULL;
	struct Chunk* it;
	for ( it = pool->chunkList; it != NULL; it = it->next)
	{
		if ( ptr > (void*)it && ptr < (void*)(it + blockSize))
		{
			result = it;
			break;
		}
	}

	return (result);
}

//----------------------------------------------------------
// Global
//----------------------------------------------------------

void *pool_malloc(struct Pool* pool)
{
	struct Node* node = NULL;

	if ( pool != NULL )
	{
		struct Chunk* chunk = findFreeChunk(pool->chunkList);

		// if there is no chunk available, create a new one.
		if ( chunk == NULL)
		{
			//printf("Found no available chunk, creating a new chunk...\n");
			chunk = createChunk(pool);
			pushChunk(pool, chunk);
		}

		// if we failed creating a new chunk, then there is nothing to do but returning null.
		if ( chunk != NULL)
		{
			node = popNode(chunk);
			//printf("Found memory (%p) from chunk (%p), popping it from the free memory stack.\n", node, chunk);
		}
	}

	return (node);
}

void pool_free(struct Pool* pool, void* ptr)
{
	if ( pool != NULL )
	{
		struct Chunk* owner = findOwner(pool->chunkList, ptr);

		if ( owner != NULL )
		{
			pushNode(owner, ptr);
		}
	}
}

void pool_delete(struct Pool* pool)
{
	if ( pool != NULL )
	{
		struct Chunk* temp = (struct Chunk*)pool->chunkList;
		struct Chunk* it;

		for ( it = temp; it != NULL; it = temp)
		{
			temp = it->next;
			//printf("\tCleaning chunk(%p)\n", it);
			free( (void*)(it) );
		}

		free(pool);
	}
}

struct Pool* pool_new(long unsigned int elementSize, long unsigned int elementPerChunk)
{
	struct Pool* pool =  malloc(sizeof(struct Pool));

	if( pool != NULL )
	{
		pool->chunkList = NULL;

		if ( elementSize >= sizeof(struct Node))
		{
			pool->elementSize = elementSize;
		}
		else
		{
			pool->elementSize = sizeof(void*);
		}

		if ( elementPerChunk > 0)
		{
			pool->elementsPerChunk = elementPerChunk;
		}
		else
		{
			free(pool);
			pool = NULL;
		}
	}

	return (pool);
}
