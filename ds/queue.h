#ifndef __QUEUE_BUFFER_H__
#define __QUEUE_BUFFER_H__

/// @brief Queue Buffer

#include <stdbool.h>
#include <stdint.h>

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



/// @brief Queue node for holding integer values
typedef struct queue_node
{
    uint32_t data;
    struct queue_node *prev, *next;
} queue_node_t;

queue_node_t* queue_node_alloc( uint32_t data );
void queue_node_dealloc( queue_node_t* node );

/// @brief Linked list based queue
typedef struct queue
{
    uint16_t size;
    queue_node_t *head, *tail;
} queue_t;

void queue_init( queue_t* queue );
void queue_append( queue_t* queue, uint32_t val );
uint32_t queue_get_top( queue_t* queue );
uint32_t queue_pop( queue_t* queue );
bool queue_empty( queue_t* queue );
void queue_clean( queue_t* queue );

#endif  // __QUEUE_BUFFER_H__