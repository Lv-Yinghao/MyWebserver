#ifndef TIME_HEAP_H
#define TIME_HEAP_H

#include <iostream>
#include <netinet/in.h>
#include <time.h>
#include <queue>
#include "../http/http_conn.h"

class http_conn;

//定时器结构体 使用堆实现
class heap_timer{
public:
    heap_timer(int delay){
        expire = delay;
    }

public:
    time_t expire;
    void (*cb_func)(http_conn*);
    http_conn* user_data;

    bool operator<(const heap_timer& a){
        return this->expire < a.expire;
    }
};

class time_heap{
public:
    time_heap() = default;
    ~time_heap(){
        while(q.size()){
            auto t = q.top();

            delete t;
            q.pop();
        }
    }

public:
    //添加定时器
    void add_timer(heap_timer* timer){
        if(!timer) return;

        q.push(timer);
    }

    //删除指定的定时器
    //仅仅将目标定时器的回调函数置为空，即所谓的延迟销毁。这将节省真正删除该定时器造成的开销
    //但是可能导致堆变大，不过这些定时器会被tick函数不定时地清理掉
    void del_timer(heap_timer* timer){
        if(!timer) return;

        timer->cb_func = nullptr;
    }

    //删除堆顶的定时器
    void pop_timer(){
        if(q.empty()) return;

        auto t = q.top();

        if(t->cb_func) t->cb_func(t->user_data);
        q.pop();

        delete t;
    }

    //调整定时器
    //删除定时器并重新添加新的定时器
    void adjust_timer(heap_timer* timer){
        if(!timer) return;
        
        add_timer(timer);
        del_timer(timer);
    }

    //获得堆顶定时器
    heap_timer* top(){
        if(q.empty()) return nullptr;
        return q.top();
    }

    //心搏函数
    //循环处理堆中到期的定时器
    void tick(){
        time_t cur = time(NULL);

        while(!q.empty()){
            auto t = q.top();

            if(t->expire > cur) break;

            pop_timer();
        }
    }

    int empty(){
        return q.empty();
    }

private:
    std::priority_queue<heap_timer*> q;
};



#endif