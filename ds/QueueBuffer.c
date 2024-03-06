#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "QueueBuffer.h"

/// @brief Instantiate the queue with the given size
/// @param queue Pointer to queue ds
/// @param size Size of queue
void QueueInit( QueueBuffer_t* queue, const int size )
{
    queue->Capacity = size;
    queue->Rear = queue->Front = queue->Size = 0;
    queue->Arr = (int*)calloc( queue->Capacity, sizeof(int) );
}

/// @brief Checks if the Queue is Empty
/// @param queue Reference to Queue Object
/// @return return's 1 if queue is empty, else 0
int QueueIsEmpty( QueueBuffer_t* queue )
{
    return queue->Size == 0;
}

/// @brief Checks if the Queue is full
/// @param queue Reference to Queue Object
/// @return return 1 if queue is full
int QueueIsFull( QueueBuffer_t* queue )
{
    return queue->Size == queue->Capacity;
}

/// @brief Insert data into the queue
/// @param queue Reference to Queue Object
/// @param data Data to insert
void QueueInsert( QueueBuffer_t* queue, const int data )
{
    if( QueueIsFull( queue ) ) 
    {
        perror("Queue Is Full\n");
        return;
    }
    // printf("\t\tQueueInsert | Inserting %d @ %d\n", data, queue->Rear );
    queue->Arr[ queue->Rear ] = data;
    queue->Rear = (queue->Rear + 1) % queue->Capacity;
    queue->Size++;
    // printf("| Next index %d\n", queue->Rear );
}

/// @brief Insert data in the queue and return the index of insertion
/// @param queue 
/// @param data 
int QueueInsertGiveIndex( QueueBuffer_t* queue, const int data )
{
    if( QueueIsFull( queue ) ) 
    {
        perror("Queue Is Full\n");
        return -1;
    }
    printf("\t\tInserting %d @ %d", data, queue->Rear );
    int index = queue->Rear;
    queue->Arr[ queue->Rear ] = data;
    queue->Rear = (queue->Rear + 1) % queue->Capacity;
    queue->Size++;
    printf("| Next index %d\n", queue->Rear );

    return index;
}

/// @brief Get the top element from the queue
/// @param queue Get the top element from the queue
/// @return returns top element
int QueueTop( QueueBuffer_t* queue )
{
    if( QueueIsEmpty( queue ) )
    {
        perror("Queue is empty\n");
        return -1;
    }

    // return top of node
    return queue->Arr[ queue->Front ];
}

/// @brief Delete top element from a queue
/// @param queue 
/// @return 
int QueuePop( QueueBuffer_t* queue )
{
    if( QueueIsEmpty( queue ) )
    {
        // perror("Queue is empty\n");
        return -1;
    }

    // return top of node
    int data = queue->Arr[ queue->Front ];
    queue->Arr[ queue->Front ] = 0;
    queue->Front = (queue->Front + 1) % queue->Capacity;
    queue->Size--;

    return data;
}

void QueueClean( QueueBuffer_t* queue )
{
    memset( queue->Arr, 0, queue->Capacity * sizeof(int) );
    queue->Rear = queue->Front = queue->Size = 0;
}

/// @brief Destroy the queue
/// @param queue Queue to be cleaned up
void QueueDestroy( QueueBuffer_t* queue )
{
    if( queue->Arr) free( queue->Arr );
    queue->Rear = queue->Front = queue->Size = 0;
}

void ViewContents( QueueBuffer_t q )
{
    printf("\n\n------------------------------------------\n");
    printf("Queue Size : %d\n", q.Size);
    printf("Queue Contents : ");
    int size = q.Size;
    while( size )
    {
        printf("%d ", q.Arr[ q.Front ]);
        q.Front = (q.Front + 1)%q.Capacity; 
        size--;
    }
    printf("\n------------------------------------------\n\n");
}