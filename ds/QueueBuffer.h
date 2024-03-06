#ifndef __QUEUE_BUFFER_H__
#define __QUEUE_BUFFER_H__

typedef struct QueueBuffer
{
    int *Arr;
    int Front;
    int Rear;
    int Size;
    int Capacity;
} QueueBuffer_t;

int QueuePop( QueueBuffer_t* queue );
int QueueTop( QueueBuffer_t* queue );
void ViewContents( QueueBuffer_t q );
void QueueClean( QueueBuffer_t* queue );
int QueueIsFull( QueueBuffer_t* queue );
int QueueIsEmpty( QueueBuffer_t* queue );
void QueueDestroy( QueueBuffer_t* queue );
void QueueInit( QueueBuffer_t* queue, const int size );
void QueueInsert( QueueBuffer_t* queue, const int data );
int QueueInsertGiveIndex( QueueBuffer_t* queue, const int data );

#endif  // __QUEUE_BUFFER_H__