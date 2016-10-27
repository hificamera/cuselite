#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_
#define BUFSIZE 8192
class TcpSocket{
public:
	int Recv();
	int Send();
	int Reply(const char* msg, int len);
	int PostRequest(const char* path, const char* host, int port, int len, const char* msg);
	void set_fd(int fd);
	char* get_body();
private:
	int m_fd;
	char m_rw_buf[BUFSIZE];
	int m_rw_length;
	int m_body_length;
	int m_head_length;
	int m_bufdata_length;
	int set_content_length();
	const char* m_http_head_tail = "\r\n\r\n";
	const char* m_http_line_tail = "\r\n";
	const char* m_http_post_head = "POST %s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"Content-Length: %d\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n\r\n";

	const char* m_empty_reply_head = "HTTP/1.1 204 No Content\r\n";
	const char* m_ok_reply_head = "HTTP/1.1 200 OK\r\n";
	const char* m_reply_head = "Content-Length: %d\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Content-Type: application/json;charset=UTF-8\r\n\r\n"
		"%s";
};
#endif
