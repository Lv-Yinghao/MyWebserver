#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include "log.h"
#include <pthread.h>
using namespace std;

Log::Log(){
    m_count = 0;
    m_is_async = false;
}

Log::~Log(){
    if(m_fp != NULL){
        fclose(m_fp);
    }
}

/*
    日志模块的初始化函数，用于设置日志文件、日志缓冲区大小、日志分割行数和阻塞队列的长度。下面对每一部分进行详细解释：

    如果设置了阻塞队列的长度，则将日志模块设置为异步模式。创建一个阻塞队列，并使用 pthread_create 函数创建一个线程，该线程将会运行 flush_log_thread 函数，用于异步写日志。

    将传入函数的参数设置为日志模块的属性，包括关闭日志标志、日志缓冲区大小、日志分割行数。

    获取当前时间，并将其存储在一个结构体中，用于生成日志文件名。

    解析文件路径，获取日志文件名。如果路径中包含 “/” 字符，则表示有目录结构，需提取出目录名和文件名分别处理，否则仅处理文件名。

    根据获取到的文件名和当前日期生成最终的日志文件名，并打开该文件。如果打开失败则返回 false。

    如果一切顺利，则返回 true。
*/
void Log::init(const char *file_name, int log_buf_size, int split_lines, int max_queue_size){
    //如果设置了max_queue_size，则设置为异步
    if(max_queue_size >= 1){
        m_is_async = true;
        m_log_queue = new block_queue<string>(max_queue_size);
        pthread_t tid;
        //flush_log_thread是回调函数，表示创建线程异步写日志
        pthread_create(&tid,NULL,flush_log_thread,NULL);
    }

    m_log_buf_size = log_buf_size;
    m_buf = new char[m_log_buf_size];
    memset(m_buf,'\0',m_log_buf_size);
    m_split_lines = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    //使用strrchr函数在file_name字符串中查找最后一个'/'字符
    const char *p = strrchr(file_name,'/');
    char log_full_name[256] = {0};

    //file_name字符串中没有'/'字符，直接使用snprintf函数将日期和文件名组成日志全名保存到log_full_name中
    if (p == NULL){
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    }
    //将'/'字符位置之后的字符保存到log_name中，将'/'字符位置之前的字符保存到dir_name中，并使用snprintf函数将目录、日期和文件名组成日志全名保存到log_full_name中。
    else{
        strcpy(log_name, p + 1);
        strncpy(dir_name, file_name, p - file_name + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }

    m_today = my_tm.tm_mday;
    m_fp = fopen(log_full_name,"a");
}


/*
    这个函数是一个日志输出函数，它接受三个参数，第一个参数level表示日志的级别，第二个参数format是一个可变参数列表，用于指定输出内容的格式，第三个参数是用于描述输出内容的其他参数。

    在函数内部，首先使用gettimeofday函数获取当前时间，然后利用localtime函数将当前时间转换为本地时间，并将转换后的时间保存在结构体my_tm中，同时将日期部分保存到成员变量m_today中。

    接着，根据传入的日志级别level，分别将日志描述符s设置成对应的字符串。如果level不在0-3范围内，则将日志描述符设置为"[info]:"。

    接下来是写入日志内容的部分。首先对m_mutex加锁，然后将日志计数器m_count加1。如果当前日期不等于当天日期（m_today != my_tm.tm_mday），或者当前日志数量是m_split_lines的倍数，那么需要将当前日志写入到一个新文件中。
    具体的处理方法是使用snprintf函数生成一个新的日志文件名，并将当前文件缓冲区中的内容刷到磁盘上，然后关闭当前文件，并使用fopen函数打开一个新的日志文件。

    接着，使用va_list和va_start函数获取可变参数列表valst，并将其传给vsnprintf函数，将格式化后的输出内容保存在m_buf中。将输出内容转换成string格式，并将其保存在log_str中。

    如果当前是异步输出模式，并且日志队列不满，那么将log_str压入日志队列中。否则，对m_mutex加锁，使用fputs函数将日志内容输出到当前日志文件中，然后对m_mutex解锁。

    最后，使用va_end函数关闭可变参数列表，并退出函数。  
*/
void Log::write_log(int level,const char *format,...){
    struct timeval now = {0,0};
    gettimeofday(&now,NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};

    switch(level){
        case 0:
            strcpy(s,"[debug]:");
            break;
        case 1:
            strcpy(s,"[info]:");
            break;
        case 2:
            strcpy(s,"[warn]:");
            break;
        case 3:
            strcpy(s,"[erro]:");
            break;
        default:
            strcpy(s,"[info]:");
            break;
    }

    //写入一个log,对m_count++,m_split_lines最大行数
    m_mutex.lock();
    m_count++;

    //写满或者日期改变后，新建新的日志文件
    if(m_today != my_tm.tm_mday || m_count % m_split_lines == 0){
        char new_log[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};

        //格式化日志名中的时间部分
        snprintf(tail,16,"%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

        //如果时间不是今天，创建今天的日志
        if(m_today != my_tm.tm_mday){
            snprintf(new_log,255,"%s%s%s",dir_name,tail,log_name);
            m_today = my_tm.tm_mday;
            m_count = 0;
        }
        else{
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, m_count / m_split_lines);
        }
        m_fp = fopen(new_log,"a");
    }

    m_mutex.unlock();

    // 可变参数列表可以用于指定输出内容的格式，并将格式化后的输出内容传递给对应的输出函数，从而实现高度灵活的日志输出。
    va_list valst;
    va_start(valst,format);

    string log_str;
    m_mutex.lock();

    //写入的具体时间内容格式
    int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
    my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
    my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);

    int m = vsnprintf(m_buf+n,m_log_buf_size-n-1,format,valst);
    m_buf[m+n] = '\n';
    m_buf[m+n+1] = '\0';
    log_str = m_buf;

    m_mutex.unlock();

    //异步写入：加入阻塞队列
    //同步写入：直接写入
    if(m_is_async && !m_log_queue->full()){
        m_log_queue->push(log_str);
    }
    else{
        m_mutex.lock();
        fputs(log_str.c_str(),m_fp);
        m_mutex.unlock();
    }
    
    va_end(valst);
}

void Log::flush(void){
    m_mutex.lock();
    //强制刷新写入流缓冲区
    fflush(m_fp);
    m_mutex.unlock();
}