#ifndef TINYBASE_QUEUES_H
#define TINYBASE_QUEUES_H

#include "tinybase-types.h"
#include "tinybase-atomic.h"

//=========================================================================
// tinybase-queues.h
//
// Module for working with queues in multithreaded code. The queues here
// are all thread-safe and lock-free.
//=========================================================================


//========================================
// Multiple Producers Single Consumer
//========================================

typedef struct mpsc_node
{
    struct mpsc_node* volatile Next;
} mpsc_node;

/* This node structure is meant to be replaced with any structure that the code wants to put
 |  in the queue. To do that, just put the first element of the structure as a volatile pointer
 |  to itself, called [Next]. Example:
|
|  struct alpha
|  {
|     struct alpha* volatile Next;
 |     int A;
 |     double B;
 |  };
|
|  Then just use it normally with the mpsc_queue below. */

typedef struct mpsc_freelist
{
    mpsc_node* volatile Head;
    mpsc_node* Tail;
    mpsc_node  Stub;
} mpsc_freelist;

/* The queue structure works as a free list. It does not actually store the data, only
 |  pointers to it. The data can be stored wherever and however, its memory layout does not
 |  matter to the queue (or to the thread-safe aspect of the code). */

external void InitMPSCFreeList(mpsc_freelist* Queue);

/* Prepares a new queue to be used. Must be called before using it.
 |--- Return: nothing. */

external void MPSCFreeListPush(mpsc_freelist* Queue, void* Item);

/* Pushes a new node [Item] into the queue. [Item] must be a structure of a sub-type of
 |  mpsc_node, as described above. There is no limit to how many nodes can be pushed into it.
 |--- Return: nothing. */

external mpsc_node* MPSCFreeListPop(mpsc_freelist* Queue);

/* Pops a node from [Queue] and returns it. Return pointer should be cast to the structure
 |  inserted in the queue.
 |--- Return: next node in the queue, or NULL if no node remaining. */


//========================================
// Multiple Producers Multiple Consumers
//========================================

typedef struct mpmc_ringbuf
{
    void** Ring;
    usz RingSize;
    usz ReadCur;
    usz WriteCur;
} mpmc_ringbuf;

/* This queue structure works as a ring buffer. [.Buf] is an array of pointers that works
|  circularly, with elements added at [.WriteCur] and read from [.ReadCur]. Both cursors wrap
|  at the end of the array, which is what makes it work circularly. [.WriteCur] will never
 |  overwrite the current [.ReadCur], nor will [.ReadCur] read past the current [.WriteCur]. */

external mpmc_ringbuf InitMPMCRingBuffer(void** Buf, usz BufSize);

/* Creates and sets up the structure. [Buf] must have been pre-allocated by the application
|  to [BufSize] number of bytes, and must be zeroed. If [BufSize] is not a power of two,
 |  this function will round it down to the nearest power.
|--- Return: initiated mpmc_ringbuf struct. */

external bool MPMCRingBufferPush(mpmc_ringbuf* Queue, void* Item);

/* Pushes a new [Item] into the queue, so long as there's space in the buffer for it.
|--- Return: true if successful, false if not. */

external void* MPMCRingBufferPop(mpmc_ringbuf* Queue);

/* Pops an item from the queue. If there are no more items to pop, it will return NULL.
|--- Return: pointer to item if successful, NULL pointer if not. */


#if !defined(TT_STATIC_LINKING)
#include "tinybase-queues.c"
#endif

#endif //TINYBASE_QUEUES_H
