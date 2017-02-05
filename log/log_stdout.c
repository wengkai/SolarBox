#include "log_stdout.h"
#include <stdio.h>

static bool open(const struct _Log_Device* this);
static bool close(const struct _Log_Device* this);
static bool output(const struct _Log_Device* this, int level, const char* message);

LogDevice log_stdout_init()
{
	LogDevice device;
	
	device.min_level = LOG_DETAIL;
	device.name = "STDOUT";
	device.open = open;
	device.close = close;
	device.output = output;

	return device;
}

static bool open(const struct _Log_Device* this)
{
	return true;
}

static bool close(const struct _Log_Device* this)
{
	return true;
}

static bool output(const struct _Log_Device* this, int level, const char* message)
{
	bool ret = true;
	if ( level >= this->min_level ) 
	{
		if ( printf("%s\n", message) <0 )
			ret = false;
	}
	return ret;
}