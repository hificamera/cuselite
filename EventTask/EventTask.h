#ifndef EVENTTASK_H_
#define EVENTTASK_H_
#include <sys/epoll.h>
#include "../mytypes.h"
class EventTask {
	public:
		EventTask(int epfd, int fd);
		virtual ~EventTask();
		virtual int getFd() = 0;
		int getEpfd();
		bool isAcceptTask();
		virtual int EpollInOutHandle(struct count_s* counter) = 0;
		void EpollRdHupErrHandle();
	protected:
		void add_fd_to_epoll();
		void del_fd_from_epoll();
		int m_fd;
		int m_epfd;
		bool m_isAcceptTask;
		struct epoll_event m_ev;
};
#endif
