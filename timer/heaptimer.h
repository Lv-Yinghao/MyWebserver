#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/log.h"
#include "../http/http_conn.h"

class http_conn;

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

class TimerNode{
public:
    int id; //对应http连接的sockfd
    time_t expires; //超时时间
    void (*cb)(http_conn*); //回调函数
    http_conn *user_data;
    bool operator<(const TimerNode& t){
        return expires <= t.expires;
    }
};

class HeapTimer{
public:
    HeapTimer(){ heap_.reserve(64); }

    ~HeapTimer(){ clear(); }

    void adjust(int id,int newExpires);

    void add(TimerNode *timer);

    void dowork(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

    void del_(int id);

    bool empty();

private:
    void siftup_(int i);

    bool siftdown_(int index,int n);

    void SwapNode_(size_t i,size_t j);

    std::vector<TimerNode> heap_;
    
    //将sockfd映射为堆数组的下标
    std::unordered_map<int,size_t> ref_;
};

#endif