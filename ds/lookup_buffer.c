#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include "lookup_buffer.h"

void lookup_buffer_init( lookup_buffer_t* buf, uint32_t size )
{
    assert(buf);
    buf->size = size;
    buf->buf = (bool*)calloc(size, sizeof(bool));
    memset( buf->buf, false, size * sizeof(bool) );
    if( pthread_rwlock_init( &buf->lock, NULL ) )
    {
        perror("Lock instantiation failed : ");
        exit(errno);
    }
}

bool lookup_buffer_set( lookup_buffer_t* buf, uint32_t idx, bool val )
{
    assert( buf );
    if( idx > buf->size )
    {
        printf("Invalid Index [%d]\n", idx);
        exit(-1);
    }
    pthread_rwlock_wrlock( &buf->lock );
    buf->buf[idx] = val;
    pthread_rwlock_unlock( &buf->lock );
}

bool lookup_buffer_get( lookup_buffer_t* buf, uint32_t idx )
{
    assert( buf );
    if( idx > buf->size )
    {
        printf("Invalid Index [%d]\n", idx);
        exit(-1);
    }
    pthread_rwlock_rdlock( &buf->lock );
    bool val = buf->buf[idx];
    pthread_rwlock_unlock( &buf->lock );
    return val;
}