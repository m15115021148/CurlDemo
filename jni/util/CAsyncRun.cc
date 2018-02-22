/**
 * file: CAsyncRun.cc
 * auto: thirchina
 * brif: POSIX thread class
 */

#ifndef _C_ASYNC_RUN_CC_
#define _C_ASYNC_RUN_CC_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "cubic_inc.h"



template <typename ArgType = void*>
class CAsyncRun
{
public:
    typedef void ( *AsyncProcFunc )( ArgType data );

private:
    typedef struct AsyncData {
        pthread_t       t_thread;
        pthread_attr_t  t_thread_attr;
        AsyncProcFunc   user_func;
        ArgType         user_data;
    } AsyncData;

    static void* static_run_proc( void* p ) {
        AsyncData* data = ( AsyncData* )p;
        RETNIF( data == NULL, NULL );
        data->user_func( data->user_data );
        delete data;
        return NULL;
    };

    static void _async_run( AsyncProcFunc func, AsyncData* data ) {
        pthread_attr_init( &data->t_thread_attr );
        pthread_attr_setdetachstate( &( data->t_thread_attr ), PTHREAD_CREATE_DETACHED );
        pthread_create( &( data->t_thread ), &( data->t_thread_attr ), CAsyncRun::static_run_proc, data );
    };

public:
    static void async_run( AsyncProcFunc func ) {
        RETIF( func == NULL );
        AsyncData* data = new AsyncData();
        RETIF( data == NULL );
        data->user_func = func;
        _async_run( func, data );
    };

    static void async_run( AsyncProcFunc func, ArgType arg ) {
        RETIF( func == NULL );
        AsyncData* data = new AsyncData();
        RETIF( data == NULL );
        data->user_func = func;
        data->user_data = arg;
        _async_run( func, data );
    };
};


#endif //_C_ASYNC_RUN_CC_

