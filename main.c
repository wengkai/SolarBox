#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "log/log.h"
#include "log/log_stdout.h"
#include "tlc1543/tlc1543.h"
#include "httpd/httpd.h"
#include "charger/charger.h"
#include "yeelink/yeelink.h"

#define PORT 14500
#define ADDR "0.0.0.0"
#define QUEUE_SIZE 20
#define BUFF_SIZE 2048

static char* _fill_in_data(char* content, int len);
// static TLC1543 _tlc1543;

int main(){
	LogDevice log_device = log_stdout_init();
	log_init(1);
	log_register_device(0, &log_device);
	log_set_level(LOG_DETAIL);

	log_output(LOG_INFO, "Server up.");

	charger_init();
	charger_start();
	
	//tlc1543_init(&_tlc1543, 0, 4000000, 0);

	httpd_init(PORT);
	httpd_set_response("", _fill_in_data);
	httpd_start();

	yl_register_apikey("98183dbe97dae0f5661d7420dcfde1ba");
	sleep(1*60);
	while (1) {
		Charger charger = charger_get_instant_data();
		yl_send_datapoint(2537, 400179, charger.vbattery);
		yl_send_datapoint(2537, 400181, charger.isolar);
		yl_send_datapoint(2537, 400188, charger.iac);
		yl_send_datapoint(2537, 400201, charger.isolar+charger.iac);
		yl_send_datapoint(2537, 400202, 
			charger.inet+charger.idesktop+charger.iradio1+charger.iradio2);
		sleep(5*60);
	}

	httpd_join();

	log_output(LOG_INFO, "Server down.");

	return 0;
}

char* _fill_in_data(char* content, int len)
{
	Charger charger = charger_get_instant_data();
	int loc=0;
	loc += snprintf(content+loc, len-loc, "battery voltage =%5.2lfV\n", 
		charger.vbattery);
	loc += snprintf(content+loc, len-loc, "solar is charging:%4.2lfA\n", 
		charger.isolar);
	loc += snprintf(content+loc, len-loc, "AC    is charging:%4.2lfA\n", 
		charger.iac);
	loc += snprintf(content+loc, len-loc, "Network is using :%4.2lfA\n", 
		charger.inet);
	loc += snprintf(content+loc, len-loc, "Desktop is using :%4.2lfA\n", 
		charger.idesktop);
	loc += snprintf(content+loc, len-loc, "Radio1	is using :%4.2lfA\n", 
		charger.iradio1);
	loc += snprintf(content+loc, len-loc, "Radio2	is using :%4.2lfA\n",
		charger.iradio2);
	loc += snprintf(content+loc, len-loc, "totally  charging:%4.2lfA\n", 
		charger.isolar + charger.iac);
	loc += snprintf(content+loc, len-loc, "totally is using :%4.2lfA\n", 
		charger.inet + charger.idesktop + charger.iradio1+charger.iradio2);
	loc += snprintf(content+loc, len-loc, "AC is %s charging.\n", 
		charger_is_ac_charging()?"":"not");
#if 0 
	int values[16] = {0};
	tlc1543_read_all(&_tlc1543, values);
	double va[16] = {0.0};
	for ( int i=0; i<14; i++ ) {
		va[i] = values[i]*5010.0/1024.0;
	}
	loc += snprintf(content+loc, len-loc, "battery voltage =%5.2lfV\n", 
			va[0]*3.175/1000);
	loc += snprintf(content+loc, len-loc, "solar is charging:%4.2lfA\n", 
			(-1)*(va[1]-2481)/100);
	loc += snprintf(content+loc, len-loc, "AC    is charging:%4.2lfA\n", 
			(-1)*(va[3]-2475)/100);
	loc += snprintf(content+loc, len-loc, "Network is using :%4.2lfA\n", 
			(-1)*(va[8]-2475)/100);
	loc += snprintf(content+loc, len-loc, "Desktop is using :%4.2lfA\n", 
			(-1)*(va[2]-2475)/100);
	loc += snprintf(content+loc, len-loc, "Radio   is using :%4.2lfA\n", 
			(-1)*(va[4]-2471)/100);
	loc += snprintf(content+loc, len-loc, "totally  charging:%4.2lfA\n", 
			(-1)*(va[3]-2475)/100+(-1)*(va[1]-2481)/100);
	loc += snprintf(content+loc, len-loc, "totally is using :%4.2lfA\n", 
			(-1)*(va[8]-2475)/100+(-1)*(va[2]-2475)/100+(-1)*(va[4]-2471)/100);
#endif
	return content;
}

#if 0
	// 建立服务器
	int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ADDR);
	server_addr.sin_port = htons(PORT);

	// 绑定 ip 以及端口
	if ( bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 ) {
		log_output(LOG_FATAL, "Count not bind %s:%d", ADDR, PORT);
		exit(1); 
	}
	if ( listen(server, QUEUE_SIZE) == -1 ) {
	    log_output(LOG_FATAL, "Listen error");
	    exit(1);
	}
	log_output(LOG_INFO, "Server running at %s:%d", ADDR, PORT);

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
		int values[16] = {0};
		tlc1543_read_all(&tlc1543, values);
		double va[16] = {0.0};
		for ( int i=0; i<14; i++ ) {
			va[i] = values[i]*5010.0/1024.0;
		}
		loc += snprintf(buf+loc, BUFF_SIZE-loc, "battery voltage=%5.2lfV\n", 
				va[0]*3.175/1000);
		loc += snprintf(buf+loc, BUFF_SIZE-loc, "solar charging current=%4.2lfA\n", 
				(-1)*(va[1]-2481)/100);
		loc += snprintf(buf+loc, BUFF_SIZE-loc, "AC charging current=%4.2lfA\n", 
				(-1)*(va[3]-2475)/100);
		loc += snprintf(buf+loc, BUFF_SIZE-loc, "Network is using=%4.2lfA\n", 
				(-1)*(va[8]-2475)/100);
		loc += snprintf(buf+loc, BUFF_SIZE-loc, "Desktop is using=%4.2lfA\n", 
				(-1)*(va[2]-2475)/100);
		loc += snprintf(buf+loc, BUFF_SIZE-loc, "Radio is using=%4.2lfA\n", 
				(-1)*(va[4]-2479)/100);
		
		
		// for ( int i=0; i<14; i++ ) {
		// 	loc += snprintf(buf+loc, BUFF_SIZE-loc, "%02d:%04d:%5.3lfV\n", 
		// 		i, values[i], va[i]);
		// }
		loc += snprintf(buf+loc, BUFF_SIZE-loc, "</pre></BODY></HTML>\n");
		log_output(LOG_DETAIL, buf);
		if ( send(conn, buf, loc, 0) == -1 ) {
			log_output(LOG_ERROR, "response fail.");
		}
		close(conn);
    }
    close(server);
    return 0;
}

#endif

// GET / HTTP/1.1
// Host: 192.168.1.22:14500
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
// Accept-Language: zh-cn
// Connection: keep-alive
// Accept-Encoding: gzip, deflate
// User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_5) AppleWebKit/601.6.17 (KHTML, like Gecko) Version/9.1.1 Safari/601.6.17
