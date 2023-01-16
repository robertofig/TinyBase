#ifndef TINYBASE_QUEUES_H
#define TINYBASE_QUEUES_H

#include "tinybase-types.h"
#include "tinybase-platform.h"

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

/* This node structure is meant to be replaced with any structure that the code
|  wants to put in the queue. To do that, just put the first element of the
|  structure as a volatile pointer to itself, called [Next]. Example:
|
|  struct alpha
|  {
|     struct alpha* volatile Next;
 |     int A;
 |     double B;
 |  };
|
|  Then just use it normally with the mpsc_queue below. */

typedef struct mpsc_queue
{
    mpsc_node* volatile Head;
    mpsc_node* Tail;
    mpsc_node  Stub;
} mpsc_queue;

/* The queue structure, works as a free list. It does not actually store the
 |  data, only pointers to it. The data can be stored wherever and however,
|  its memory layout does not matter to the queue (or to the thread-safe
|  aspect of the code). */

external void InitMPSCQueue(mpsc_queue* Queue);

/* Prepares a new queue to be used. Must be called before using it.
 |--- Return: nothing. */

external void MPSCQueuePush(mpsc_queue* Queue, void* Element);

/* Pushes a new node [Element] into the queue. [Element] must be a structure
|  of a sub-type of mpsc_node, as described above. There is no limit to
|  how many nodes can be pushed into it.
 |--- Return: nothing. */

external mpsc_node* MPSCQueuePop(mpsc_queue* Queue);

/* Pops a node from [Queue] and returns it. Return pointer should be cast
|  to the structure inserted in the queue.
 |--- Return: next node in the queue, or NULL if no node remaining. */


#if !defined(TT_STATIC_LINKING)
#include "tinybase-queues.c"
#endif

#endif //TINYBASE_QUEUES_H
