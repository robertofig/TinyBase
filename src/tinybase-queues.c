
external void
InitMPSCQueue(mpsc_queue* Queue)
{
    Queue->Head = &Queue->Stub;
    Queue->Tail = &Queue->Stub;
}

external void
MPSCQueuePush(mpsc_queue* Queue, void* Element)
{
    mpsc_node* Node = (mpsc_node*)Element;
    Node->Next = NULL;
    mpsc_node* Prev = (mpsc_node*)AtomicExchangePtr((void* volatile*)&Queue->Head, Node);
    Prev->Next = Node;
}

external mpsc_node*
MPSCQueuePop(mpsc_queue* Queue)
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
    
    MPSCQueuePush(Queue, &Queue->Stub);
    Next = Tail->Next;
    
    if (Next)
    {
        Queue->Tail = Next;
        return Tail;
    }
    
    return NULL;
}
