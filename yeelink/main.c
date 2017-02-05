#include <stdio.h>
#include "yeelink.h"
#include "../log/log.h"
#include "../log/log_stdout.h"

int main()
{
	LogDevice log_device = log_stdout_init();
	log_init(1);
	log_register_device(0, &log_device);
	log_set_level(LOG_DETAIL);

	yl_register_apikey("98183dbe97dae0f5661d7420dcfde1ba");
	yl_send_datapoint(2537, 400179, 12.0);
}