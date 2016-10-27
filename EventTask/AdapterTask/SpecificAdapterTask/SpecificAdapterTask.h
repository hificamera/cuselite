#ifndef SPECIFICADAPTERTASK_H_
#define SPECIFICADAPTERTASK_H_
#include "../AdapterTask.h"
class SpecificAdapterTask : public AdapterTask {
	public:
		SpecificAdapterTask(int epfd, int fd);
		virtual ~SpecificAdapterTask();
	protected:
		Task_ReturnValue sendResponse();//需要各自分别实现
		Task_ReturnValue getRequestPbByJson(char* request_str, int len);//需要各自分别实现
		void copyandreplaceunderline(char* dst, const std::string& src, size_t len, char c);
		const char* m_trackurl = "http://m.admixture.com.cn:8080/dsp-monitor/2000/%s?info=%s_%s_0_%s_%s__%4d%02d%02d%02d%02d%02d_%d_cpm_%lf_0_0_0__0_0___&c=%%%%WIN_PRICE%%%%&guid=&sign=&apid=%s";
};
#endif
