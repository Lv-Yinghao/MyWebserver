Demo演示
注册演示

TinyWebServer
Linux下C++轻量级Web服务器，助力初学者快速实践网络编程，搭建属于自己的服务器.

使用 线程池 + 非阻塞socket + epoll(ET和LT均实现) + 事件处理(Reactor和模拟Proactor均实现) 的并发模型
使用状态机解析HTTP请求报文，支持解析GET和POST请求
访问服务器数据库实现web端用户注册、登录功能，可以请求服务器图片和视频文件
实现同步/异步日志系统，记录服务器运行状态
经Webbench压力测试可以实现上万的并发连接数据交换

登录演示
![image](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/1f59a6cc-807c-4c8e-8453-ec2fc21b5bcf)


请求图片文件演示(6M)

请求视频文件演示(39M)

压力测试
在关闭日志后，使用Webbench对服务器进行压力测试，对listenfd和connfd分别采用ET和LT模式，均可实现上万的并发连接，下面列出的是两者组合后的测试结果.

Proactor，LT + LT，93251 QPS

Proactor，LT + ET，97459 QPS

Proactor，ET + LT，80498 QPS

Proactor，ET + ET，92167 QPS

Reactor，LT + ET，69175 QPS

并发连接总数：10500
访问服务器时间：5s
所有访问均成功
注意： 使用本项目的webbench进行压测时，若报错显示webbench命令找不到，将可执行文件webbench删除后，重新编译即可。

