#include "httpd.h"
#include "../log/log.h"

#include <pthread.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 14500
#define ADDR "0.0.0.0"
#define QUEUE_SIZE 20
#define BUFF_SIZE 2048

static void *_httpd_server(void *arg);

static int _httpd_port = PORT;
static char *(*_httpd_response_func)(char* content, int len) =0;
static pthread_t _httpd_thread_id;

int httpd_init(int port)
{
	_httpd_port = port;
	return 0;
}

int httpd_set_response(const char* url, char *(*response_func)(char* content, int len))
{
	_httpd_response_func = response_func;
	return 0;
}

int httpd_start(void)
{
	int ret = 0;
	int err = pthread_create(&_httpd_thread_id, NULL, _httpd_server, NULL);
	if ( err ) 
    { 
        log_output(LOG_FATAL, "can't create server thread:%s", strerror(err)); 
        ret = -1;
    }
    return ret;
}

void httpd_join(void)
{
	pthread_join(_httpd_thread_id, NULL);
}

void *_httpd_server(void *arg)
{
	// 建立服务器
	int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ADDR);
	server_addr.sin_port = htons(_httpd_port);

	// 绑定 ip 以及端口
	if ( bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 ) {
		log_output(LOG_FATAL, "Could not bind %s:%d", ADDR, _httpd_port);
		exit(1); 
	}
	if ( listen(server, QUEUE_SIZE) == -1 ) {
	    log_output(LOG_FATAL, "Listen error");
	    exit(1);
	}
	log_output(LOG_INFO, "Server running at %s:%d", ADDR, _httpd_port);

	while (1){
	    struct sockaddr_in clientAddr;
	    socklen_t length = sizeof(clientAddr);
		// 对连入的连接进行处理
		log_output(LOG_INFO, "waiting...");
		int conn = accept(server, (struct sockaddr*)&clientAddr, &length); 
		if ( conn < 0 ) {
			log_output(LOG_FATAL, "Connect error");
			exit(1); 
		}
		log_output(LOG_INFO, "A new connection from %s:%d", 
			inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
		// 处理连接发送过来的字符
	    char buf[BUFF_SIZE];
	    int count;
	    memset(buf, 0, sizeof(buf));
		// 接收字符
		count = recv(conn, buf, sizeof(buf), 0);
		// 如果接收到的字符为空,则表示离开 
		if (count == 0){
			close(conn);
			continue;
		}
		log_output(LOG_INFO, "%s", buf);
		int loc = snprintf(buf, BUFF_SIZE, 
			"HTTP/1.1 200 OK\n"
			"Connection: close\n"
			"Content-Type: text/html; charset=utf-8\n"
			"\n"
			"<HTML><BODY><pre>\n");
		// loc += snprintf(buf+loc, BUFF_SIZE-loc, "TEST\n");
		if ( _httpd_response_func ) {
			char content[BUFF_SIZE];
			_httpd_response_func(content, BUFF_SIZE-loc);
			loc += snprintf(buf+loc, BUFF_SIZE-loc, "%s", content);
		}
		loc += snprintf(buf+loc, BUFF_SIZE-loc, "</pre></BODY></HTML>\n");
		log_output(LOG_DETAIL, buf);
		if ( send(conn, buf, loc, 0) == -1 ) {
			log_output(LOG_ERROR, "response fail.");
		}
		close(conn);
    }
    close(server);
}