#ifndef ADAPTERTASK_H_
#define ADAPTERTASK_H_
#include <sys/time.h>
#include "../EventTask.h"
#include "../../utils/socket/TcpSocket.h"
#include "../../protocol/dsp-adapter.pb.h"
class AdapterTask : public EventTask {
	public:
		AdapterTask(int epfd, int fd);
		virtual ~AdapterTask();
		virtual int getFd();
		virtual int EpollInOutHandle(struct count_s* counter);		
		enum Task_Status {
			GETREQUESTINFO = 0x001,
			SENDTORTB = 0x002,
			RECVFROMRTB = 0x004,
			SENDRESPONSE = 0x008,
			COLLECTREQUEST = 0x010,
			WITHERROR = 0x100,
			DONE = 0x800

		};
		enum Task_ReturnValue{
			TASK_BLOCK = 0,
			TASK_GOON = 1,
			TASK_ERROR = 2
		};
	protected:
		void FinishTask(struct count_s* counter);
		Task_ReturnValue getRequestInfo();
		Task_ReturnValue sendToRtb();
		Task_ReturnValue prepareToSendToRtb();
		Task_ReturnValue recvFromRtb();
		virtual Task_ReturnValue sendResponse() = 0;//需要各自分别实现
		virtual Task_ReturnValue getRequestPbByJson(char* request_str, int len) = 0;//需要各自分别实现
		void sendEmptyResponse();

		int m_status;
		int m_fd_for_rtb;
		int m_fd_now;
		struct timeval m_tv_begin;
		struct timeval m_tv_end;
		TcpSocket m_tcp_sock;
		char m_rw_buf[8192];
		qgadapter::AdRequest m_requestPb;
		qgadapter::AdResponse m_responsePb;

};
#endif
