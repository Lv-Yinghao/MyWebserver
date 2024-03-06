

# TinyWebServer

## 项目简介：

本项目是制作的一个简单的Linux系统服务器项目，涉及到的知识点包括网络编程，Linux系统使用，服务器性能调优，C/C++编程，计算机网络等多个领域的知识，同时本项目是我第一次对一个项目进行系统测试，找到了项目的弊端并基于了修正，程序的健壮性和可维护性有了进一步的提高。

下面是我所做的一些主要工作

- 使用 线程池 + 非阻塞socket + epoll(ET和LT均实现) 的并发模型

- 使用状态机解析HTTP请求报文，支持解析GET和POST请求
- 访问服务器数据库实现web端用户注册、登录功能，可以请求服务器图片和视频文件
- 实现同步/异步日志系统，记录服务器运行状态
- 经Webbench压力测试可以实现上万的并发连接数据交换

## Demo演示

### 注册演示

![注册演示](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/ef83ba53-5c4d-49a4-9151-a8222d419307)


### 登录演示

![登录演示](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/27e2a483-9620-4359-940f-4ce38b802055)


### 请求图片文件演示

![请求图片演示](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/29ae889a-ec9f-43e1-b331-4f93adfdafcf)

### 请求视频文件演示

![请求视频演示](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/dbcf2036-fa45-4a56-a757-cfad37b7457a)

## 测试环节

本项目的测试包括功能测试，性能测试，安全性测试，兼容性测试和易用性测试以及UI测试六部分。主要的测试目标在于项目的登录和注册以及访问资源这两项和核心功能。下图是我绘制的测试用例的一份思维导图。

![Webserver测试用例](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/74766295-5fd8-44db-8156-236fcf17424a)

### 功能测试

对于功能测试、兼容性测试以及安全性测试的测试用例，我使用了web端自动化测试工具selenuim搭配pytest自动化测试框架实现，同时采用了yaml文件实现数据驱动。在完成首次测试目标后，使用PO设计模式对其进行封装。

> PO是Page Object的缩写，PO模式是自动化测试项目开发实践的最佳设计模式之一。 核心思想是通过对界面元素的封装减少冗余代码，同时在后期维护中，若元素定位发生变化， 只 需要调整页面元素封装的代码，提高测试用例的可维护性、可读性。 

本项目使用PO模式可以把一个页面分为三层，对象库层、操作层、业务层。分别实现其相应的功能。

- 对象库层：封装定位元素的方法。 
- 操作层：封装对元素的操作。
- 业务层：将一个或多个操作组合起来完成一个业务功能。比如登录：需要输入帐号、密码、点 击登录三个操作。

### 接口测试

接口测试主要针对页面跳转的情况，对其返回的结果进行断言，判断各种情况下页面能够正常跳转到正常的页面。

### 测试报告

接口测试结果：

![接口测试结果](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/62705229-6c32-4c7f-bf38-c6acaa782dbf)

登录功能测试结果：

![登录UI自动化测试结果](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/c79f7146-5eb4-4e91-98b1-cc12cee6103e)

注册功能测试结果：

![注册UI自动化测试结果](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/7cba5515-9a06-4221-99fa-4f441a088541)

采用allure插件输出测试报告：

![allure测试报告](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/52d8a97e-da8b-4952-a533-00ba31f4e094)

### 压力测试

压力测试是专门的一个模块，对于Web项目，通常比较关心并发量和响应时间这两个性能指标。

有关浏览器响应时间的测试，可以根据浏览器的开发者模式中直接观察到具体结果，所有响应的耗时都在10ms以内，并发量提高响应时间并无明显变化。

![响应时间](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/1f0ef4ef-bcb1-47ef-87bc-d590ccaeb2ee)

有关并发量的测试，这里采用的工具是Webbench，使用起来非常简单。

在关闭日志后，使用Webbench对服务器进行压力测试，对监听的文件描述符分别采用ET和LT模式，测试结果如下：

LT模式测试结果：

![LT测试结果](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/273ba0a6-a118-4adc-af5e-6c0bcce19c13)

ET模式测试结果：

![ET测试结果](https://github.com/Lv-Yinghao/MyWebserver/assets/76142149/1f900478-5357-47de-8906-05a6eee97ff2)

可以发现相较于LT模式，ET模式的并发有明显提高。


