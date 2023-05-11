#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"

using namespace std;

class Log{
public:
    //C++ 11以后，使用局部静态变量懒汉不用加锁
    static Log *get_instance(){
        static Log instance;
        return &instance;
    }

    static void *flush_log_thread(void *args){
        Log::get_instance()->async_write_log();
    }
    
    //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
    void init(const char *file_name, int log_buf_size, int split_lines, int max_queue_size);

    void write_log(int level,const char *format,...);

    void flush(void);

private:
    Log();

    virtual ~Log();

    void *async_write_log(){
        string single_log;
        //从阻塞队列中取出一个日志，写入文件
        while(m_log_queue->pop(single_log)){
            m_mutex.lock();
            fputs(single_log.c_str(),m_fp);
            m_mutex.unlock();
        }
    }

private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines; //日志最大行数
    int m_log_buf_size; //日志缓冲区大小
    long long m_count; //日志行数记录
    int m_today; //按天分类
    FILE *m_fp; //打开log的文件指针
    char *m_buf; //缓冲区
    block_queue<string> *m_log_queue; //阻塞队列
    bool m_is_async; //是否同步标志位
    locker m_mutex; //互斥锁
};

/*
    在定义宏时使用可变参数(…)，如果后面的参数个数不唯一，那么使用##VA_ARGS 来代表所有可变参数，
    这个符号会在可变参数非空时将逗号前面的内容（即上一个参数）和后面的所有参数连接。
    如果可变参数为空，会将前面的逗号去掉，避免出现难看的格式问题。
    在这里，它的作用就是将format和后面的可变参数用逗号链接起来，传到Log类中的write_log方法中去。
*/
#define LOG_DEBUG(format, ...) Log::get_instance()->write_log(0, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) Log::get_instance()->write_log(1, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) Log::get_instance()->write_log(2, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Log::get_instance()->write_log(3, format, ##__VA_ARGS__)

#endif