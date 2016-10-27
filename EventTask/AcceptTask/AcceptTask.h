#ifndef ACCEPTTASK_H_
#define ACCEPTTASK_H_
#include "../EventTask.h"
class AcceptTask : public EventTask {
public:
	AcceptTask(int epfd, int fd);
	virtual ~AcceptTask();
	virtual int getFd();
	virtual int EpollInOutHandle(struct count_s* counter);		
};
#endif
