#include "log.h"
#include "log_stdout.h"
#include <stdio.h>

int main(void)
{
	LogDevice log_device = log_stdout_init();
	log_init(1);
	log_register_device(0, &log_device);
	log_set_level(LOG_DETAIL);
	for ( int i=0; i<100; i++ ) {
		log_output(LOG_INFO, "%d", i);
	}
	log_close();
	return 0;
}
