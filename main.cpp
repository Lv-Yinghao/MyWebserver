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
#include "locker.h"
#include "threadpool.h"
#include "sort_timer_lst.h"

#define MAX_FD 65536   // 最大的文件描述符个数
#define MAX_EVENT_NUMBER 10000  // 监听的最大的事件数量

static sort_timer_lst timer_lst;        //定时器链表
static int epollfd = 0;
static int pipefd[2];

// 添加文件描述符
extern void addfd( int epollfd, int fd, bool one_shot );
// 移除文件描述符
extern void removefd( int epollfd, int fd );
// 设置文件描述符为非阻塞
extern int setnonblocking( int fd );

void ignore_sig(int sig, void( handler )(int)){
    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) );
    sa.sa_handler = handler;
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1 );
}

//倒计时结束触发回调函数后，往管道中写触发的信号
void sig_handler(int sig){
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1],(char*)&msg,1,0);
    errno = save_errno;
}

void addsig(int sig){
    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) );
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1 );
}

void timer_handler(){
    //定时处理任务
    timer_lst.tick();
    alarm(TIMESLOT);
}

//定时器回调函数：删除非活跃连接socket上的注册事件，并关闭
void cb_func(http_conn* user_data){
    epoll_ctl(epollfd,EPOLL_CTL_DEL,user_data->m_sockfd,0);
    close((user_data->m_sockfd));
    // printf("close fd %d\n",user_data->m_sockfd);
}

int main( int argc, char* argv[] ) {
    
    if( argc <= 1 ) {
        printf( "usage: %s port_number\n", basename(argv[0]));
        return 1;
    }

    int port = atoi( argv[1] );
    ignore_sig( SIGPIPE, SIG_IGN );

    threadpool< http_conn >* pool = NULL;
    try {
        pool = new threadpool<http_conn>;
    } catch( ... ) {
        return 1;
    }

    http_conn* users = new http_conn[ MAX_FD ];

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
    addfd(epollfd,pipefd[0],true);

    //设置信号处理函数
    addsig(SIGALRM);
    addsig(SIGTERM);

    //SIGTERM信号终止进程
    bool stop_server = false;

    // client_data* users = new client_data[FD_LIMIT];
    //有没有超时事件
    bool timeout = false;
    
    //alarm函数设置的闹钟一旦超时就会产生超时信号SIGALRM，我们利用该信号的信号处理函数俩处理定时任务
    alarm(TIMESLOT);

    while(!stop_server) {
        
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
        
        if ( ( number < 0 ) && ( errno != EINTR ) ) {
            printf( "epoll failure\n" );
            break;
        }

        for ( int i = 0; i < number; i++ ) {
            
            int sockfd = events[i].data.fd;
            
            if( sockfd == listenfd ) {
                
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                
                if ( connfd < 0 ) {
                    printf( "errno is: %d\n", errno );
                    continue;
                } 

                // 用户数量过多，关闭最新的用户连接
                if( http_conn::m_user_count >= MAX_FD ) {
                    close(connfd);
                    continue;
                }

                users[connfd].init(connfd, client_address);
                
                printf("sockfd为%d的连接成功建立......\n",connfd);

                //定时器相关

                //定时器的初始化
                util_timer *timer = new util_timer;
                timer->user_data = &users[connfd];
                timer->cb_func = cb_func;

                //设置超时时间为当前时间 + 15s
                time_t cur = time(NULL);
                timer->expire = cur + 3 * TIMESLOT;
                
                //初始化用户连接的定时器
                users[connfd].timer = timer;

                //在双向升序链表中添加定时器 尾部
                timer_lst.add_timer(timer);
            } else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {
                //执行定时器回调函数并从链表中删除定时器
                cb_func(&users[sockfd]);
                timer_lst.delete_timer(users[sockfd].timer);

                users[sockfd].close_conn();
            
            } else if((sockfd == pipefd[0]) && (events[i].events & EPOLLIN)){
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
                util_timer* timer_ = new util_timer;
                time_t cur = time(NULL);
                timer_->expire = cur + 3*TIMESLOT;
                timer_lst.adjust_timer(timer_);

                if(users[sockfd].read()) {
                    pool->append(users + sockfd);
                } else {
                    //执行定时器回调函数并从链表中删除定时器
                    cb_func(&users[sockfd]);
                    timer_lst.delete_timer(users[sockfd].timer);

                    users[sockfd].close_conn();
                }

            }  else if( events[i].events & EPOLLOUT ) {

                if( !users[sockfd].write() ) {
                    //执行定时器回调函数并从链表中删除定时器
                    cb_func(&users[sockfd]);
                    timer_lst.delete_timer(users[sockfd].timer);

                    users[sockfd].close_conn();
                }

            }
        }

        if(timeout){
            timer_handler();
            timeout = false;
        }
    }

    close( listenfd );
    close( pipefd[0] );
    close( pipefd[1] );
    close( epollfd ); 
    
    delete [] users;
    delete pool;

    return 0;
}