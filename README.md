# cuselite
项目中原有高并发框架，是主流的，主线程accept，线程池做具体处理。

本项目是一个尝试，通过epoll，以及虚函数，使得不再区分主次线程，任何一个线程都有可能做任何工作，简化代码结构，如果需要添加功能，则继承EventTask实现自己的Task，该Task需有一个非阻塞的fd.