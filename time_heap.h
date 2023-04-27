#pragma once

#include <iostream>
#include <netinet/in.h>
#include <time.h>
#include "http_conn.h"
using std::exception;

#define buffer_size 64

class http_conn;

class heap_timer{
public:
    heap_timer(int delay){
        expire = time(NULL) + delay;
    }

public:
    time_t expire;
    void (*cb_func)(http_conn*);
    http_conn* user_data;
};




