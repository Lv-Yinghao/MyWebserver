#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>

#include "./lock/locker.h"
#include "./threadpool/threadpool.h"
#include "./timer/heaptimer.h"
#include "./http/http_conn.h"
#include "./log/log.h"
#include "./CGImysql/sql_connection_pool.h"

// #define SYNLOG //同步写日志
#define ASYNLOG //异步写日志

// #define listenfdET //边缘触发非阻塞
#define listenfdLT //水平触发阻塞

#define MAX_FD 65536   // 最大的文件描述符个数
#define MAX_EVENT_NUMBER 10000  // 监听的最大的事件数量

//定时器相关参数
static HeapTimer heap;                     //最小堆实现的定时器
static int epollfd = 0;
static int pipefd[2];

//下面三个函数的具体定义在http_conn.cpp中

extern void addfd( int epollfd, int fd, bool one_shot );

extern void removefd( int epollfd, int fd );

extern int setnonblocking( int fd );

extern void modfd(int epollfd, int fd, int ev);

//倒计时结束触发回调函数后，往管道中写触发的信号
void sig_handler(int sig){
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1],(char*)&msg,1,0);
    errno = save_errno;
}

//设置信号函数
void addsig(int sig,void(handler)(int),bool restart = true){
    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) );
    sa.sa_handler = handler;
    if(restart) sa.sa_flags |= SA_RESTART;
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1 );
}

//定时处理任务，重新定时以不断触发，SIGALRM信号
void timer_handler(){
    int num = heap.GetNextTick();
    // printf("下一个时间是num = %d\n",num);
    modfd( epollfd, pipefd[0], EPOLLIN );

    //堆不为空
    if(num > 0) alarm( num );
}

//定时器回调函数：删除非活跃连接socket上的注册事件，并关闭
void cb_func(http_conn* user_data){
    // printf("sockfd为%d的连接已断开......\n",user_data->m_sockfd);
    user_data->close_conn();
    LOG_INFO("close fd %d",user_data->m_sockfd);
    Log::get_instance()->flush();
}

void show_error(int connfd,const char *info){
    printf("%s",info);
    send(connfd,info,strlen(info),0);
    close(connfd);
}

int main( int argc, char* argv[] ) {
#ifdef ASYNLOG
    Log::get_instance()->init("ServerLog",2000,800000,8); //异步日志模型
#endif

#ifdef SYNLOG
    Log::get_instance()->init("ServerLog",2000,800000,0); //同步日志模型
#endif

    if( argc <= 1 ) {
        printf( "usage: %s port_number\n", basename(argv[0]));
        return 1;
    }

    int port = atoi( argv[1] );
    addsig( SIGPIPE, SIG_IGN );

    //创建数据库连接池
    connection_pool *connPool = connection_pool::GetInstance();
    connPool->init("localhost","root","123456","yourdb",3306,8);

    //创建线程池
    threadpool< http_conn >* pool = NULL;
    try {
        pool = new threadpool<http_conn>(connPool);
    } catch( ... ) {
        return 1;
    }

    http_conn* users = new http_conn[ MAX_FD ];

    //初始化数据库读取表
    users->initmysql_result(connPool);

    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );

    int ret = 0;
    struct sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons( port );

    // 端口复用
    int reuse = 1;
    setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );

    // 将socketfd和地址绑定，也叫给socket命名
    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    // 创建监听队列存放被监听的socket
    ret = listen( listenfd, 5 );

    // 创建epoll对象，和事件数组，添加
    epoll_event events[ MAX_EVENT_NUMBER ];
    epollfd = epoll_create( 5 );
    // 添加到epoll对象中
    addfd( epollfd, listenfd, false );
    http_conn::m_epollfd = epollfd;

    //创建管道 通过往管道中写数据出发epoll内核事件
    ret = socketpair(PF_UNIX,SOCK_STREAM,0,pipefd);
    //设置写非阻塞
    setnonblocking(pipefd[1]);
    //将管道的读端加入内核事件表中
    addfd(epollfd,pipefd[0],false);

    //设置信号处理函数
    addsig(SIGALRM,sig_handler, false);
    addsig(SIGTERM,sig_handler, false);

    //SIGTERM信号终止进程
    bool stop_server = false;

    //有没有超时事件
    bool timeout = false;

    while(!stop_server) {

        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1);
        
        if ( ( number < 0 ) && ( errno != EINTR ) ) {
            LOG_ERROR("%s","epoll failure");
            break;
        }

        for ( int i = 0; i < number; i++ ) {
            
            int sockfd = events[i].data.fd;
            
            if( sockfd == listenfd ) {
                // printf("监听到新的连接......\n");

                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
#ifdef listenfdLT //监听文件描述符 水平触发
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                
                if ( connfd < 0 ) {
                    LOG_ERROR("%s:errno is %d","accept error",errno);
                    continue;
                } 

                if( http_conn::m_user_count >= MAX_FD ) {
                    show_error(connfd,"Internal server busy");
                    LOG_ERROR("%s","Internal server busy");
                    continue;
                }
           
                users[connfd].init(connfd, client_address);

                // printf("sockfd为%d的连接已建立......\n",connfd);

                //定时器的初始化
                TimerNode *timer = new TimerNode();
                time_t cur = time(NULL);
                
                timer->id = connfd;
                timer->user_data = &users[connfd];
                timer->expires = cur + 3*TIMESLOT;
                timer->cb = cb_func;

                users[connfd].timer = timer;

                if(heap.empty()) alarm(3*TIMESLOT);

                heap.add(timer);

#endif

#ifdef listenfdET //监听文件描述符 边沿触发
                while(1){
                    int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                
                    if ( connfd < 0 ) {
                        LOG_ERROR("%s:errno is %d","accept error",errno);
                        break;
                    } 

                    if( http_conn::m_user_count >= MAX_FD ) {
                        show_error(connfd,"Internal server busy");
                        LOG_ERROR("%s","Internal server busy");
                        break;
                    }
            
                    users[connfd].init(connfd, client_address);
                    
                    TimerNode *timer = new TimerNode();
                    time_t cur = time(NULL);
                    
                    timer->id = connfd;
                    timer->user_data = &users[connfd];
                    timer->expires = cur + 3*TIMESLOT;
                    timer->cb = cb_func;

                    users[connfd].timer = timer;

                    if(heap.empty()) alarm(3*TIMESLOT);

                    heap.add(timer);
                }
                continue;
#endif
            } else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {
                //执行定时器回调函数并从链表中删除定时器
                // printf("exception: sockfd为%d的文件描述符有新情况，被迫断开连接......\n",sockfd);
                cb_func(&users[sockfd]);
                heap.del_(sockfd);
        
            } else if((sockfd == pipefd[0]) && (events[i].events & EPOLLIN)){
                // printf("处理管道的读事件\n");
                int sig;
                char signals[1024];
                ret = recv(pipefd[0],signals,sizeof(signals),0);
                if(ret == -1 || ret == 0) continue;
                else{
                    for(int i = 0; i < ret; ++i){
                        switch(signals[i]){
                            case SIGALRM:
                                //记录超时事件，优先处理IO操作
                                timeout = true;
                                break;
                            case SIGTERM:
                                //终止进程
                                stop_server = true;
                        }
                    }
                }
            
            } else if(events[i].events & EPOLLIN) {
                // printf("read: sockfd为%d的文件描述符有新的写事件\n",sockfd);
                if(users[sockfd].read()) {
                    //inet_ntoa用于将一个32位的IP地址从二进制格式转换成点分十进制格式
                    // puts("写成功......");
                    LOG_INFO("deal with the client(%s)",inet_ntoa(users[sockfd].m_address.sin_addr));
                    Log::get_instance()->flush();
                    pool->append(users + sockfd);

                    LOG_INFO("%s","adjust timer once");
                    Log::get_instance()->flush();
                    heap.adjust(sockfd,2*TIMESLOT);
                } 
                else {
                    cb_func(&users[sockfd]);
                    heap.del_(sockfd);
                }

            }  else if( events[i].events & EPOLLOUT ) {
                // printf("write: sockfd为%d的文件描述符有新的读事件\n",sockfd);
                if( !users[sockfd].write() ) {
                    cb_func(&users[sockfd]);
                    heap.del_(sockfd);
                }
                else{
                    // puts("读成功......");
                    LOG_INFO("send data to the client(%s)",inet_ntoa(users[sockfd].m_address.sin_addr));
                    Log::get_instance()->flush();

                    LOG_INFO("%s","adjust timer once");
                    Log::get_instance()->flush();
                    heap.adjust(sockfd,3*TIMESLOT);
                }
            }
        }

        if(timeout){
            timer_handler();
            timeout = false;
        }
    }

    close( listenfd );
    close( epollfd );

    close( pipefd[0] );
    close( pipefd[1] );
    
    delete [] users;
    delete pool;

    return 0;
}