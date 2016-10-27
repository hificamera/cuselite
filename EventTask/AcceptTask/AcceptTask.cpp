#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <glog/logging.h>
#include <fcntl.h>
#include "AcceptTask.h"
#include "../AdapterTask/SpecificAdapterTask/SpecificAdapterTask.h"
AcceptTask::AcceptTask(int epfd, int fd) : EventTask(epfd, fd){
	m_isAcceptTask = true;
}
AcceptTask::~AcceptTask(){
}

int AcceptTask::getFd(){
	return m_fd;
}
int AcceptTask::EpollInOutHandle(struct count_s* counter){
	//执行accept
	//这里未对accept用的fd 如果出现问题的情况 进行处理,可以考虑如果该fd由于某种情况确定不可用了,重新生成fd 以及对应的 AcceptTask实例, 并且重新放入epoll
	int fd_new = 0;
	int flags = 0;
	int res = 0;
	struct epoll_event ev;
	ev.events = EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLONESHOT;
	fd_new = accept(m_fd, nullptr, nullptr);
	if(-1 == fd_new){
		if(errno != EAGAIN && errno != EWOULDBLOCK)
			LOG(ERROR) << "accept error, the fd is " << getFd() << ". The errno is " << errno << ".";

		return 0;
	}
	flags = fcntl(fd_new, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(fd_new, F_SETFL, flags);
	ev.data.ptr = new SpecificAdapterTask(m_epfd, fd_new);
	res = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd_new, &ev);
	if(-1 == res){
		LOG(ERROR) << "accept add " << fd_new << " error";
	}
	return 0;
}
