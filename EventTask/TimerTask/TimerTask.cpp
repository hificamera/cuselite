#include <glog/logging.h>
#include <iomanip>
#include <ctime>
#include "TimerTask.h"

TimerTask::TimerTask(int epfd, int fd, struct count_s* counter, unsigned short thread_num) : EventTask(epfd, fd), m_counter(counter), m_thread_num(thread_num){
	m_isAcceptTask = true;
	m_count_times = 0;
	reset();
}
TimerTask::~TimerTask(){
}
void TimerTask::reset(){
	if(m_out_ofstream.is_open())
		m_out_ofstream.close();
	//m_count_times = 0;
	m_count_request = 0;
	m_count_response = 0;
	m_count_response_by_other = 0;
	m_count_empty_response = 0;
	m_count_error_request = 0;
	m_count_error_rtb = 0;

	time_t nowtime = time(nullptr) + 60 * 60 * 8;
	struct tm gmt_time;
	gmtime_r(&nowtime, &gmt_time);
	strftime(m_filename, 32, "Statistic/%Y%m%d.txt", &gmt_time);

	m_out_ofstream.open(m_filename, std::ios::out | std::ios::app);
	if(m_out_ofstream.fail()){
		LOG(ERROR) << "open statistic file error";
	}
}
int TimerTask::getFd(){
	return m_fd;
}
int TimerTask::EpollInOutHandle(struct count_s* counter){
	time_t nowtime = time(nullptr) + 60 * 60 * 8;
	struct tm gmt_time;
	gmtime_r(&nowtime, &gmt_time);
	char t[16];
	strftime(t, 16, "%H:%M:%S", &gmt_time);
	/*
	if(0 == gmt_time.tm_hour && 0 == gmt_time.tm_min)
	*/
	reset();

	read(m_fd, &m_utt, sizeof(m_utt));
	for(unsigned short i = 0; i < m_thread_num; ++i){
		m_count_request += m_counter[i].count_request.exchange(0);
		m_count_response += m_counter[i].count_response.exchange(0);
		m_count_response_by_other += m_counter[i].count_response_by_other.exchange(0);
		m_count_empty_response += m_counter[i].count_empty_response.exchange(0);
		m_count_error_request += m_counter[i].count_error_request.exchange(0);
		m_count_error_rtb += m_counter[i].count_error_rtb.exchange(0);
	}
	if(!m_out_ofstream.is_open()){
		m_out_ofstream.open(m_filename, std::ios::out | std::ios::app);
		if(m_out_ofstream.fail()){
			LOG(ERROR) << "open statistic file error";
			return 0;
		}
	}
	if(0 == m_count_times % 10){
		m_out_ofstream << std::setw(8) << "截止时间" << "\t" << std::setw(13) << "请求数量" << "\t" << std::setw(11) << "响应数量" << "\t" << std::setw(12) << "仙果响应数量" << "\t" << std::setw(10) << "空响应数量" << "\t" << std::setw(8) << "请求错误" << "\t" << std::setw(15) << "RTB请求广告错误" << std::endl;
	}
	m_out_ofstream << std::setw(8) << t << "\t" << std::setw(13) << m_count_request << "\t" << std::setw(11) << m_count_response << "\t" << std::setw(12) << m_count_response_by_other << "\t" << std::setw(10) << m_count_empty_response << "\t" << std::setw(8) << m_count_error_request << "\t" << std::setw(15) << m_count_error_rtb << std::endl;
	//++m_count_times;
	m_count_times = (m_count_times + 1) % 10;
}
