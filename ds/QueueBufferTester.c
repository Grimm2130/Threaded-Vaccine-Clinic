#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "QueueBuffer.h"


int main( void )
{
    int size = 100;
    QueueBuffer_t q;

    QueueInit( &q, size );

    for( int i = 0; i < size; i++ )
    {
        int data = rand() % 100;
        QueueInsert( &q, data );
    }

    assert( q.Size == size );
    printf("Insertion Complete\n");

    ViewContents( q );

    int removeCount = 10, addCount = 10;
    size -= removeCount;

    while( removeCount )
    {
        int data = QueuePop( &q );
        printf("\tDeleting %d\n", data);
        removeCount--;
    }

    printf("\tSize after deletion : %d\nComparing to %d to %d\n", q.Size, q.Size, size);
    assert( q.Size == size );

    for( int i = 0; i < addCount; i++ )
    {
        int data = rand() % 100;
        QueueInsert( &q, data );
    }
    size += addCount;
    printf("\tSize after addition : %d\nComparing to %d to %d\n", q.Size, q.Size, (size-removeCount));
    assert( q.Size == size );
    ViewContents( q );

    printf("Done!\n");
    return 0;
}