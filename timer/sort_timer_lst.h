// #ifdef LST_TIMER
// #define LST_TIMER
#pragma once

#include <time.h>
#include "../http/http_conn.h"

using namespace std;

#define BUFFER_SIZE 64
class http_conn;

// struct client_data{
//     sockaddr_in addresss; //客户端地址 
//     int sockfd; //客户端socket地址
//     util_timer* timer; //为每一个客户端分配一个定时器
// };


//定时器
class util_timer{
public:
    util_timer():prev(NULL),next(NULL){}

public:
    //任务超时时间
    time_t expire;
    //任务回调函数
    void (*cb_func)(http_conn* user_data);
    //回调函数处理的用户数据
    http_conn* user_data;
    util_timer* prev; //指向前一个定时器
    util_timer* next; //指向后一个定时器
};

//定时器双向升序链表
class sort_timer_lst{
public:
    sort_timer_lst() : head(NULL),tail(NULL){}
    //链表销毁时，删除所有的定时器
    ~sort_timer_lst(){
        util_timer* tmp = head;
        while(tmp){
            head = tmp->next;
            delete tmp;
            head = tmp;
        }
    }
    //添加定时器
    /*
        如果目标定时器的超时时间小于当前链表中所有定时器的超时时间，则把该定时器插入链表头部，作为链表新的头结点
        否则需要调用重载函数将其插入到链表中的合适位置，保证链表的升序特性
    */
    void add_timer(util_timer* timer){
        if(!timer) return;
        if(!head){
            head = tail = timer;
            return;
        }
        if(timer->expire < head->expire){
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer,head);
    }
    //调整定时器
    void delete_timer(util_timer* timer){
        if(!timer) return;
        //链表中只有一个定时器
        if(timer == head && timer == tail){
            delete timer;
            head = tail = NULL;
            return;
        }
        //目标定时器是链表的头结点
        if(timer == head){
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        //目标定时器是链表的尾结点
        if(timer == tail){
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        //目标定时器是链表的中间节点
        timer->next->prev = timer->prev;
        timer->prev->next = timer->next;
        delete timer;
    }

    //调整定时器在链表中的位置
    void adjust_timer(util_timer* timer){
        if(!timer) return;
        util_timer* tmp = timer->next;
        
        //链表尾部或者是超时时间仍然小于下一个定时器，不做调整
        if(!tmp || timer->expire < tmp->expire) return;

        //链表头部，重置表头，重新添加节点
        if(timer == head){
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer,head);
        }
        else{
            //链表中间部分，重置所在位置，重新添加节点
            timer->next->prev = timer->prev;
            timer->prev->next = timer->next;
            add_timer(timer,timer->next);
        }
    }

    //SIGALRM信号每当触发一次就在其信号处理函数中执行一次tick函数，处理链表上到期的任务
    void tick(){
        if(!head) return;

        printf("time tick\n");
        
        //获取系统当前时间
        time_t cur = time(NULL);
        util_timer* tmp = head;
        
        //从头结点开始一次处理每一个定时器，直到遇到一个尚未到期的定时器，这是定时器的核心逻辑
        while(tmp){
            //每个定时器使用绝对时间作为超时值，所以我们可以将定期是的超时值和系统当前时间作比较判断定期器是否过期
            if(cur < tmp->expire) break;
            
            //定时器回调函数
            tmp->cb_func(tmp->user_data);

            //删除定时器
            head = tmp->next;
            if(head) head->prev = NULL;
            delete tmp;
            tmp = head;
        }
    }

    //将定时器添加到链表lst_head之后的位置中
    void add_timer(util_timer* timer,util_timer* lst_head){
        util_timer* prev = lst_head;
        util_timer* tmp = prev->next;

        while(tmp){
            if(timer->expire < tmp->expire){
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = tmp;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }

        if(!tmp){
            prev->next = timer;
            timer->next = NULL;
            timer->prev = prev;
            tail = timer;
        }
    }


private:
    util_timer * head;
    util_timer * tail;
};

// #endif