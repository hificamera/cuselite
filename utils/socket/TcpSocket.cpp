#include <cstring>
#include "TcpSocket.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <cstdio>
void TcpSocket::set_fd(int fd){
	m_fd = fd;
	//fd更改时，需要将buf置0
	m_rw_length = 0;
	m_head_length = 0;
	m_body_length = 0;
	m_bufdata_length = 0;
	memset(m_rw_buf, 0, BUFSIZE);
}

int TcpSocket::Recv(){
	int res = recv(m_fd, m_rw_buf + m_rw_length, BUFSIZE - 1 - m_rw_length, 0);
	if(-1 == res){
		if(EAGAIN == errno || EWOULDBLOCK == errno){
			return 0;
		}
		return -1;
	}else if (0 == res){
		return -1;
	}
	m_rw_length += res;
	if(0 == m_body_length){
		res = set_content_length();
		if(-1 == res){
			return -1;
		}
	}
	if(m_rw_length - m_head_length < m_body_length){
		return 0;
	}
	return m_body_length;
}

int TcpSocket::set_content_length(){
	/*
	if(strncmp(m_rw_buf, "POST", strlen("POST"))){
		return -1;
	}
	*/

	char* p_temp = strstr(m_rw_buf, m_http_head_tail);
	if(nullptr == p_temp){
		return -1;
	}
	m_head_length = p_temp - m_rw_buf + strlen(m_http_head_tail);

	//暂时不考虑chunked编码
	p_temp = strstr(m_rw_buf, "Content-Length:");
	if(nullptr == p_temp){
		return -1;
	}

	m_body_length = atoi(p_temp + strlen("Content-Length:"));
	if(0 == m_body_length){
		return -1;
	}
	if(m_head_length + m_body_length > BUFSIZE){
		return -1;
	}
	return 0;
}

char* TcpSocket::get_body(){
	return m_rw_buf + m_head_length;
}

int TcpSocket::Reply(const char* msg, int len){
	if(0 == m_bufdata_length){
		if(len < 0){
			return -1;
		}else if(0 == len){
			sprintf(m_rw_buf, "%s", m_empty_reply_head);
		}else{
			sprintf(m_rw_buf, "%s", m_ok_reply_head);
		}
		m_bufdata_length = strlen(m_rw_buf);
		sprintf(m_rw_buf + m_bufdata_length, m_reply_head, len, msg);
		m_bufdata_length = strlen(m_rw_buf);
	}

	return Send();
}

int TcpSocket::PostRequest(const char* path, const char* host, int port, int len, const char* msg){
	if(0 == m_bufdata_length){
		sprintf(m_rw_buf, m_http_post_head, path, host, port, len);
		m_bufdata_length = strlen(m_rw_buf) + len;
		memcpy(m_rw_buf + strlen(m_rw_buf), msg, len);
	}

	return Send();
}

int TcpSocket::Send(){
	int res = send(m_fd, m_rw_buf + m_rw_length, m_bufdata_length - m_rw_length, 0);
	if(-1 == res){
		if(EAGAIN == errno || EWOULDBLOCK == errno){
			return 0;
		}
		return -1;
	}
	m_rw_length += res;
	if(m_rw_length < m_bufdata_length){
		return 0;
	}
	m_rw_length = 0;
	return m_bufdata_length;
}
