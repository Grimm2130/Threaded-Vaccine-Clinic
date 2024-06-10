#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "queue.h"

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

// ---------------------------- Queue Instantiation ----------------------------

static void node_detach( queue_node_t* node )
{
    queue_node_t *p = node->prev, *n = node->next;
    if( p) p->next = n;
    if(n) n->prev = p;
    node->prev = node->next = NULL;
}

queue_node_t* queue_node_alloc( uint32_t data )
{
    queue_node_t* newNode = (queue_node_t*) calloc(1, sizeof(queue_node_t));
    newNode->next = newNode->prev = NULL;
    newNode->data = data;
    return newNode;
}

void queue_node_dealloc( queue_node_t* node )
{
    assert(node);
    node_detach( node );
    free( node );
}

void queue_init( queue_t* queue )
{
    assert( queue );
    queue->head = queue->tail = NULL;
    queue->size = 0;
}

void queue_append( queue_t* queue, uint32_t val )
{
    assert(queue);
    queue_node_t* temp = queue_node_alloc( val );
    if( !queue->head )
    {
        assert( !queue->size );
        queue->head = queue->tail = temp;
    }
    else
    {
        queue->tail->next = temp;
        temp->prev = queue->tail;
        queue->tail = temp;
    }
    queue->size++;
}

uint32_t queue_get_top( queue_t* queue )
{
    assert( queue );
    if( queue_empty(queue) ) exit(-1);
    return queue->head->data;
}

uint32_t queue_pop( queue_t* queue )
{
    assert( queue );
    if( queue_empty(queue) ) exit(-1);
    queue_node_t* temp = queue->head;
    queue->head = queue->head->next;
    queue->size--;
    assert( queue->size >= 0 );
    if( queue->size == 0 )
    {
        queue->head = queue->tail = NULL;
    }
    int val = temp->data;
    queue_node_dealloc( temp );
    temp = NULL;
    return val;
}

bool queue_empty( queue_t* queue )
{
    assert(queue);
    return queue->size == 0;
}

void queue_clean( queue_t* queue )
{
    queue_node_t* curr = queue->head;
    while( curr )
    {
        queue->head = queue->head->next;
        queue_node_dealloc( curr );
        curr = queue->head;
    }
}