#include<unistd.h>
#include<glog/logging.h>
#include<cstdio>
#include<cerrno>
#include "EventTask.h"
EventTask::EventTask(int epfd, int fd) : m_epfd(epfd), m_fd(fd){
}

EventTask::~EventTask(){
	close(m_fd);
}

int EventTask::getEpfd(){
	return m_epfd;
}

void EventTask::EpollRdHupErrHandle(){
	del_fd_from_epoll();
	delete this;
}

bool EventTask::isAcceptTask(){
	return m_isAcceptTask;
}

void EventTask::del_fd_from_epoll(){
	int res = epoll_ctl(m_epfd, EPOLL_CTL_DEL, getFd(), nullptr);
	if(-1 == res)
		LOG(ERROR) << "EPOLL_CTL_DEL error: [" << errno << "]";
}

void EventTask::add_fd_to_epoll(){
	int res = epoll_ctl(m_epfd, EPOLL_CTL_ADD, getFd(), &m_ev);
	if(-1 == res)
		LOG(ERROR) << "EPOLL_CTL_ADD error: [" << errno << "]";
}
