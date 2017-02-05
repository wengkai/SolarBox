#ifndef _LOG_H_
#define _LOG_H_

#include <stdbool.h>

enum {
	LOG_DETAIL,
	LOG_INFO,
	LOG_ERROR,
	LOG_FATAL,
	NUMBER_OF_LEVELS
} LogLevel;

typedef struct _Log_Device {
	bool is_registered;
	bool is_enabled;
	int min_level;
	int file_descriptor;
	char *name;

	bool (*open)(const struct _Log_Device* this);
	bool (*close)(const struct _Log_Device* this);
	bool (*output)(const struct _Log_Device* this, int level, const char* message);
} LogDevice;

bool log_init(int number_of_devices);
void log_close();
bool log_register_device(int device_id, const LogDevice* p_log_device);
void log_set_level(int level);
void log_output(int level, const char* format, ...);

#endif