#ifndef TIMERTASK_H_
#define TIMERTASK_H_
#include <fstream>
#include "../EventTask.h"
#include "../../mytypes.h"
class TimerTask : public EventTask {

public:
	TimerTask(int epfd, int fd, struct count_s* counter, unsigned short thread_num);
	virtual ~TimerTask();
	virtual int EpollInOutHandle(struct count_s* counter);		
	virtual int getFd();
private:
	void reset();
	struct count_s* m_counter;
	uint64_t m_utt;
	unsigned short m_count_times;
	unsigned short m_thread_num;
	unsigned long m_count_request;
	unsigned long m_count_response;
	unsigned long m_count_response_by_other;
	unsigned long m_count_empty_response;
	unsigned long m_count_error_request;
	unsigned long m_count_error_rtb;
	char m_filename[32];
	std::ofstream m_out_ofstream;
};
#endif
