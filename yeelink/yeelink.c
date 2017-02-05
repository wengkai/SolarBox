#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "../log/log.h"

#define KEY_LEN 80
#define BUF_LEN 1024
#define HOST_NAME "api.yeelink.net"
#define HTTP_PORT 80

static char API_KEY[KEY_LEN] = {0};

void yl_register_apikey(const char* apikey)
{
	strncpy(API_KEY, apikey, KEY_LEN);
}

int yl_send_datapoint(
	int device_id, int datapoint_id, double value)
{  
    int result_id;  
    int socket_id; 
    int ret = 0; 
    
    char value_data[BUF_LEN];
    snprintf(value_data, BUF_LEN, "{\"value\":%lf}\r\n", value);
    
    // HTTP请求  
    char send_buf[BUF_LEN];
    snprintf(send_buf, BUF_LEN, 
	    "POST /v1.0/device/%d/sensor/%d/datapoints HTTP/1.1\r\n"  
	    "U-ApiKey: %s\r\n" 
	    "Accept:*/*\r\n"
	    "Host: api.yeelink.net\r\n"
	    "Content-Length:%d\r\n"
	    "Content-Type:application/x-www-form-urlencoded\r\n"
	    "\r\n%s",
	    device_id, datapoint_id ,
	    API_KEY, 
	    strlen(value_data), value_data);
    // HTTP响应  
    char receive_buf[BUF_LEN];  
    struct hostent *yeelink_host; // yeelink主机DNS解析结构体  
    // char *host_name = "api.yeelink.net"; // yeelink域名  
    struct in_addr yeelink_ipaddr; // yeelink IP地址  
    struct sockaddr_in yeelink_sockaddr; // yeelink 连接套接字  
     
    // 第一步 DNS地址解析  
    //printf("calling gethostbyname with: %s\r\n", HOST_NAME);  
    yeelink_host = gethostbyname(HOST_NAME);  
    if (yeelink_host == NULL) {  // DNS解析失败  
        log_output(LOG_FATAL, "Can not resolve host%s",HOST_NAME);  
        ret = -1;
        goto EXIT;
    } 
    yeelink_ipaddr.s_addr = *(unsigned long *) yeelink_host->h_addr_list[0];  
    log_output(LOG_DETAIL, "%s IP Address %s" , 
    	HOST_NAME, inet_ntoa(yeelink_ipaddr));  
    
    // 第二步 创建套接字  
    socket_id = socket(AF_INET, SOCK_STREAM, 0);  
    yeelink_sockaddr.sin_family = AF_INET;  
    yeelink_sockaddr.sin_port = htons(HTTP_PORT); // 设置端口号  
    yeelink_sockaddr.sin_addr = yeelink_ipaddr; // 设置IP地址  
    memset(&(yeelink_sockaddr.sin_zero), 0, sizeof(yeelink_sockaddr.sin_zero));  
    // 第三步 连接yeelink  
    result_id = connect( socket_id, (struct sockaddr *)&yeelink_sockaddr, sizeof(struct sockaddr));  
    if( result_id == -1 ) {
    	log_output(LOG_FATAL, "Can not connect to %s", HOST_NAME);  
    	ret = -1;
    	goto EXIT;
    }  
    log_output(LOG_DETAIL, "Http request:%s",send_buf);  
     
    // 第四步 发送HTTP请求  
    result_id = send(socket_id , send_buf,strlen(send_buf), 0);  
    if( result_id == -1 ) {
    	log_output(LOG_FATAL, "Can not send to %s", HOST_NAME);  
    	ret = -1;
    	goto CLOSE;
    }
    // 第五步 接收HTTP响应  
    int bytes_received = 0;  
    bytes_received = recv( socket_id , receive_buf , 1024 , 0);  
    if( bytes_received == -1 ) {
    	log_output(LOG_FATAL, "Can get respond from %s", HOST_NAME);  
    	ret = -1;
    	goto CLOSE;
    }  
    receive_buf[ bytes_received ] = '\0';  
    log_output(LOG_DETAIL, "Receive Message:%s",receive_buf );  

CLOSE:
    close(socket_id); // 关闭套接字  
EXIT:
    return ret;
}  