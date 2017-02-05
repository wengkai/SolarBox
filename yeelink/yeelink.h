#ifndef _YEELINK_H_
#define _YEELINK_H_

void yl_register_apikey(const char* apikey);

int yl_send_datapoint(
	int device_id, int datapoint_id, double value);

#endif