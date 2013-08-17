/*
 * pool.h
 *
 *  Created on: 4 janv. 2013
 *      Author: Matthieu
 */

#ifndef POOL_H_
#define POOL_H_

struct Pool
{
	long unsigned int elementSize;
	long unsigned int elementsPerChunk;

	void* chunkList;
};

void* pool_malloc(struct Pool* pool);

void pool_free(struct Pool* pool, void* ptr);

struct Pool* pool_new(long unsigned int unitSize, long unsigned int defaultElementsCount);

void pool_delete(struct Pool* pool);

#endif /* POOL_H_ */
