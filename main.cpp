#include <stdio.h>
#include <csignal>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <strings.h>
#include <thread>
#include <mutex>
#include <hash_map>
#include <inttypes.h>
#include <glog/logging.h>
#include <sys/timerfd.h>
#include "mytypes.h"
#include "EventTask/AcceptTask/AcceptTask.h"
#include "EventTask/TimerTask/TimerTask.h"
#define EVNUMS 200
#define THREADNUMS 10
std::mutex epollMutex;
//线程函数
void thread_task(int epfd, struct count_s* counter, int thread_index){
	std::unique_lock<std::mutex> epollLock = std::unique_lock<std::mutex>(epollMutex, std::defer_lock);
	struct epoll_event events[EVNUMS];
	int res = 0;
	while(true){
		epollLock.lock();
		int n = epoll_wait(epfd, events, EVNUMS, -1);
		epollLock.unlock();
		for (int i = 0; i < n; ++i){
			EventTask* pet = (EventTask*)(events[i].data.ptr);
			res = pet->EpollInOutHandle(&(counter[thread_index]));
		}
	}
}

//main函数中创建监听fd以及定时器fd，加入epoll，并启动threads
int main(int argc, char* argv[]){

	signal(SIGPIPE, SIG_IGN);

	//创建监听socket并加入epoll监听
	auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd) {
		perror("socket"), exit(-1);
	}
	int flags;
	flags = fcntl(sockfd, F_GETFL, 0);
	if (flags < 0){
		perror("F_GETFL");
	}
	flags |= O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flags);

	int reuseaddr = 1;
	int res = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
	if(res < 0){
		perror("setsockopt");
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = inet_addr("0.0.0.0");

	res = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	if(-1 == res){
		perror("bind"), exit(-1);
	}
	LOG(INFO) << "bind success!";

	res = listen(sockfd, 255);
	if(-1 == res){
		perror("listen"), exit(-1);
	}
	LOG(INFO) << "listen success!";

	int epfd = epoll_create(10);
	struct epoll_event ev;
	ev.data.ptr = new AcceptTask(epfd, sockfd);//AcceptTask应该设计成单例模式
	ev.events = EPOLLIN;

	res = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
	if(-1 == res){
		perror("epoll_ctl"), exit(-1);
	}
	LOG(INFO) << "epoll add sockfd success!";

	//创建定时计数器，加入epoll监听
	struct count_s counter[THREADNUMS];
	memset(counter, 0, sizeof(counter));

	int tfd = timerfd_create(CLOCK_REALTIME, 0);
	if(-1 == tfd)
		perror("timerfd_create"), exit(-1);
	struct itimerspec itimer;
	itimer.it_interval.tv_sec = 60;
	itimer.it_interval.tv_nsec = 0;
	itimer.it_value.tv_sec = 1;
	itimer.it_value.tv_nsec = 0;
	res = timerfd_settime(tfd, 0, &itimer, nullptr);
	if(-1 == res)
		perror("timer_settime"), exit(-1);

	struct epoll_event tfdev;//event for timerfd
	tfdev.data.ptr = new TimerTask(epfd, tfd, counter, THREADNUMS);
	tfdev.events = EPOLLIN|EPOLLET;
	res = epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &tfdev);
	if(-1 == res)
		perror("epoll_ctl"), exit(-1);

	std::thread work_threads[THREADNUMS];
	for (int i = 0; i < THREADNUMS; i++)
		work_threads[i] = std::thread(thread_task, epfd, counter, i);
	for (auto& t: work_threads)
		t.join();

	return EXIT_SUCCESS;
}
