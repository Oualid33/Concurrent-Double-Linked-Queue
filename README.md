# Concurrent-Double-Linked-Queue
A simple fine-grained lock-based program for concurrent enqueuing/dequeuing on shared double-linked queue 

This program implements a double queue data structure where 'enqueue' and 'dequeue' operations are executed 
conccurrently by two diffrent types of threads: enqueuers and dequeuers.

A queue is defined by its HEAD and TAIL: HEAD: where we DEQUEUE, and TAIL where we ENQUEUE.
Initially, the queue is empty. 
 Enqueuers and dequeuers attempt conccurrently to access to the queue to acomplish enqueue and dequeue operations (no assumption on the arrival of threads).

We use TWO fine-grained lock:
Deadlock situations are possible !

Three different cases are to be considered:
1. head != tail != NULL: normal case with no synch. issues -->
      enqueuers and dequeuers are decoupled form each others;
2. head == tail: two sub-cases:
     2.1. head == tail == NULL: problem in enqueuers, and no problem in dequeuers
     2.2. head == tail != NULL (list with one element): no problem in enqueuers and problem in dequeuers

