#include<sys/epoll.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<fcntl.h>
#include<inttypes.h>
#include<hash_map>
//#include<iostream>
#include "SpecificAdapterTask.h"
#include "../../../utils/json/JsonTools.h"
extern __gnu_cxx::hash_map<uint32_t, uint32_t> media_map;
SpecificAdapterTask::SpecificAdapterTask(int epfd, int fd) : AdapterTask(epfd, fd) {
}

SpecificAdapterTask::~SpecificAdapterTask(){
}

AdapterTask::Task_ReturnValue SpecificAdapterTask::sendResponse(){
	return TASK_GOON;
}
AdapterTask::Task_ReturnValue SpecificAdapterTask::getRequestPbByJson(char* body, int len){
	JsonTools req_json = JsonTools(body, len);
	if(!(req_json.done)){
		return TASK_ERROR;
	}

	/*
	 * 这里是具体处理的解析过程
	 */
	return TASK_GOON;
}
