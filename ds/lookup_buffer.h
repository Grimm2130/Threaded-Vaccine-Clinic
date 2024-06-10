#ifndef __LOOKUP_BUFFER_H__
#define __LOOKUP_BUFFER_H__

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>

typedef struct lookup_buffer
{
    bool *buf;
    uint32_t size;
    pthread_rwlock_t lock;
}lookup_buffer_t;

void lookup_buffer_init( lookup_buffer_t* buf, uint32_t size );
bool lookup_buffer_set( lookup_buffer_t* buf, uint32_t idx, bool val );
bool lookup_buffer_get( lookup_buffer_t* buf, uint32_t idx );

#endif  // __LOOKUP_BUFFER_H__