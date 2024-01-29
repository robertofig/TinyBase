//================
// MPSC Free List
//================

external void
InitMPSCFreeList(mpsc_freelist* Queue)
{
    Queue->Head = &Queue->Stub;
    Queue->Tail = &Queue->Stub;
}

external void
MPSCFreeListPush(mpsc_freelist* Queue, void* Item)
{
    mpsc_node* Node = (mpsc_node*)Item;
    Node->Next = NULL;
    mpsc_node* Prev = (mpsc_node*)AtomicExchangePtr((void* volatile*)&Queue->Head, Node);
    Prev->Next = Node;
}

external mpsc_node*
MPSCFreeListPop(mpsc_freelist* Queue)
{
    mpsc_node* Tail = Queue->Tail;
    mpsc_node* Next = Tail->Next;
    
    if (Tail == &Queue->Stub)
    {
        if (Next == NULL) return NULL;
        
        Queue->Tail = Next;
        Tail = Next;
        Next = Next->Next;
    }
    
    if (Next)
    {
        Queue->Tail = Next;
        return Tail;
    }
    
    mpsc_node* Head = Queue->Head;
    if (Tail != Head) return NULL;
    
    MPSCFreeListPush(Queue, &Queue->Stub);
    Next = Tail->Next;
    
    if (Next)
    {
        Queue->Tail = Next;
        return Tail;
    }
    
    return NULL;
}


//==================
// MPMC Ring Buffer
//==================

external mpmc_ringbuf
InitMPMCRingBuffer(void** Ring, usz RingSize)
{
    mpmc_ringbuf Result = {0};
    if (Ring)
    {
        RingSize = RoundDownToPow2(RingSize);
        Result.Ring = Ring;
        Result.MaxCur = (RingSize / sizeof(void*)) - 1;
    }
    return Result;
}

external bool
MPMCRingBufferPush(mpmc_ringbuf* Queue, void* Item)
{
    if (AtomicCompareExchangePtr(&Queue->Ring[Queue->WriteCur & Queue->MaxCur], 0, Item))
    {
        AtomicAddFetchIsz(&Queue->WriteCur, 1);
        return true;
    }
    return false;
}

external void*
MPMCRingBufferPop(mpmc_ringbuf* Queue)
{
    void* Item = Queue->Ring[Queue->ReadCur & Queue->MaxCur];
    if (Item
        && AtomicCompareExchangePtr(&Queue->Ring[Queue->ReadCur & Queue->MaxCur], Item, 0))
    {
        AtomicAddFetchIsz(&Queue->ReadCur, 1);
        return Item;
    }
    return 0;
}
