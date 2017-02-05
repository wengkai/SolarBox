#ifndef _HTTPD_HEAD_
#define _HTTPD_HEAD_

int httpd_init(int port);

int httpd_set_response(const char* url, char *(*response_func)(char* content, int len));

int httpd_start(void);

void httpd_join(void);

#endif