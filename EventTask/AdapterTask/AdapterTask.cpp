#include<sys/epoll.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<fcntl.h>
#include<inttypes.h>
#include "AdapterTask.h"
#include<hash_map>
#include <glog/logging.h>
#include "../../utils/json/JsonTools.h"
AdapterTask::AdapterTask(int epfd, int fd) : EventTask(epfd, fd) {
	m_tcp_sock.set_fd(m_fd);
	m_fd_now = m_fd;
	m_fd_for_rtb = 0;
	m_status = GETREQUESTINFO;
	m_ev.data.ptr = this;
}
AdapterTask::~AdapterTask(){
	if (m_fd_for_rtb > 0){
		close(m_fd_for_rtb);
	}
}

int AdapterTask::getFd(){
	return m_fd_now;
}

int AdapterTask::EpollInOutHandle(struct count_s* counter){
	int res = 0;
	del_fd_from_epoll();
	uint32_t ev_in = EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLONESHOT;
	uint32_t ev_out = EPOLLOUT|EPOLLET|EPOLLRDHUP|EPOLLONESHOT;
	//判断状态决定具体执行函数
	if(m_status & GETREQUESTINFO){
		res = getRequestInfo();
		if(res == TASK_BLOCK){
			m_fd_now = m_fd;
			m_ev.events = ev_in;
			add_fd_to_epoll();
			return 0;
		}else if(res == TASK_ERROR){
			m_status |= WITHERROR;
			counter->count_error_request++;
		}else{
			counter->count_request++;
			res = prepareToSendToRtb();
			if(TASK_BLOCK == res){
				m_fd_now = m_fd_for_rtb;
				m_ev.events = ev_out;
				add_fd_to_epoll();
				return 0;
			}else if(TASK_ERROR == res){
				m_status |= WITHERROR;
				counter->count_error_rtb++;
			}
		}
	}
	if(m_status & SENDTORTB){
		res = sendToRtb();
		if(res == TASK_BLOCK){
			m_fd_now = m_fd_for_rtb;
			m_ev.events = ev_out;
			add_fd_to_epoll();
			return 0;
		}else if(res == TASK_ERROR){
			m_status |= WITHERROR;
			counter->count_error_rtb++;
		}else{
			m_status = RECVFROMRTB;
			m_tcp_sock.set_fd(m_fd_for_rtb);
		}
	}
	if(m_status & RECVFROMRTB){
		res = recvFromRtb();
		if(res == TASK_BLOCK){
			m_fd_now = m_fd_for_rtb;
			m_ev.events = ev_in;
			add_fd_to_epoll();
			return 0;
		}else if(res == TASK_ERROR){
			m_status |= WITHERROR;
			counter->count_error_rtb++;
		}else{
			m_status = SENDRESPONSE;
			m_tcp_sock.set_fd(m_fd);
		}
	}
	if(m_status & SENDRESPONSE){
		res = sendResponse();
		if(res == TASK_BLOCK){
			m_fd_now = m_fd;
			m_ev.events = ev_out;
			add_fd_to_epoll();
			return 0;
		}else if(res == TASK_ERROR){
			m_status |= WITHERROR;
			LOG(ERROR) << "send response error.";
		}else {
			counter->count_response++;
			if(7936 != m_responsePb.rtbid()){
				counter->count_response_by_other++;
			}
			m_status = DONE;
		}
	}
	//每完成一项任务 则将 m_status改为需要执行的下一个任务的状态，有不能继续执行task的错误出现则 位或 DONE 或者  位或 WITHERROR 
	FinishTask(counter);
	return 0;
}

AdapterTask::Task_ReturnValue AdapterTask::prepareToSendToRtb(){
	int res = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == res){
		return TASK_ERROR;
	}
	m_fd_for_rtb = res;
	int flags = fcntl(m_fd_for_rtb, F_GETFL, 0);
	if(-1 == flags){
		return TASK_ERROR;
	}
	flags |= O_NONBLOCK;
	res = fcntl(m_fd_for_rtb, F_SETFL, flags);
	if(-1 == res){
		return TASK_ERROR;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80/*m_port_for_rtb*/);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");//这里的数字都应该出现在配置文件里
	res = connect(m_fd_for_rtb, (struct sockaddr*)&addr, sizeof(addr));
	if(-1 == res){
		if(EINPROGRESS == errno){
			m_tcp_sock.set_fd(m_fd_for_rtb);
			m_status = SENDTORTB;
			return TASK_BLOCK;
		}else{
			return TASK_ERROR;
		}
	}
	m_tcp_sock.set_fd(m_fd_for_rtb);
	m_status = SENDTORTB;
	return TASK_GOON;
}

void AdapterTask::FinishTask(struct count_s* counter){
	if (m_status & WITHERROR){
		//发回空Response 发回空 应该不需要计较 是否返回了错误
		sendEmptyResponse();
		counter->count_empty_response++;
	}
	delete this;
}

void AdapterTask::sendEmptyResponse(){
	m_tcp_sock.Reply(nullptr, 0);
}

AdapterTask::Task_ReturnValue AdapterTask::recvFromRtb(){
	int res = m_tcp_sock.Recv();//待修改
	if(0 == res){
		return TASK_BLOCK;
	}else if(-1 == res){
		return TASK_ERROR;
	}

	if(!m_responsePb.ParseFromArray(m_tcp_sock.get_body(), res))
		return TASK_ERROR;

	return TASK_GOON;
}

AdapterTask::Task_ReturnValue AdapterTask::getRequestInfo(){
	int res = m_tcp_sock.Recv();
	//recv或者send之后 将 m_rw_length 置0
	if(0 == res){
		return TASK_BLOCK;
	}else if(-1 == res){
		return TASK_ERROR;
	}
	return getRequestPbByJson(m_tcp_sock.get_body(), res);
}

AdapterTask::Task_ReturnValue AdapterTask::sendToRtb(){
	int res = m_tcp_sock.PostRequest("/req", "127.0.0.1", 80, m_requestPb.ByteSize(), m_rw_buf);//配置文件中提取
	if(0 == res){
		return TASK_BLOCK;
	}else if(-1 == res){
		return TASK_ERROR;
	}
	return TASK_GOON;
}

