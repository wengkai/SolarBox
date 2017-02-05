#include "log.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <strings.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef BUF_SIZE
#define BUF_SIZE 400
#endif

static LogDevice *_aDevices = 0;
static int _number_of_devices = 0;
static int _current_level = 0;

static void _log(int level, const char* message);

//	--------------------------------

bool log_init(int number_of_devices)
{
	assert(number_of_devices > 0);
	_aDevices = (LogDevice*)malloc(sizeof(LogDevice)*number_of_devices);
	if ( _aDevices != 0 ) {
		bzero(_aDevices, sizeof(LogDevice)*number_of_devices);
		_number_of_devices = number_of_devices;
		return true;
	}
	return false;
}

void log_close()
{
	for ( int i=0; i<_number_of_devices; i++ ) {
		if ( _aDevices[i].is_registered ) {
			(*_aDevices[i].close)(&(_aDevices[i]));
		}
	}
	free(_aDevices);
	_aDevices = 0;
	_number_of_devices = 0;
}

bool log_register_device(int device_id, const LogDevice* p_log_device)
{
	assert(device_id > -1 && device_id < _number_of_devices);
	if ( p_log_device->open(p_log_device) ) {
		_aDevices[device_id] = *p_log_device;
		_aDevices[device_id].is_registered = true;
		_aDevices[device_id].is_enabled = true;
		return true;
	}
	return false;
}

void log_set_level(int level)
{
	assert(level>-1 && level<NUMBER_OF_LEVELS);
	_current_level = level;
}

void log_output(int level, const char* format, ...)
{
	static const char* TYPES[] = {
		"DETAIL",
		"INFO",
		"ERROR",
		"FATAL",
	};
	char buf[BUF_SIZE];
	assert(level>-1 && level<NUMBER_OF_LEVELS);
	if ( level >= _current_level ) {
		time_t now = time(0);
		struct tm* pCurrentTime = localtime(&now);
		int len = snprintf(buf, BUF_SIZE, "%s:%04d%02d%02d-%02d%02d%02d:", TYPES[level],
			pCurrentTime->tm_year+1900, pCurrentTime->tm_mon+1, pCurrentTime->tm_mday,
			pCurrentTime->tm_hour, pCurrentTime->tm_min, pCurrentTime->tm_sec);
		va_list ap;
		va_start(ap, format);
		vsnprintf(buf+len, BUF_SIZE-len, format, ap);
		_log(level, buf);

	}
}

//  -----------------------------------------------

static void _log(int level, const char* message)
{
	//assert(level > -1 && level < NUMBER_OF_LEVELS);
	for ( int i=0; i<_number_of_devices; i++ ) {
		if ( _aDevices[i].is_registered && _aDevices[i].is_enabled &&
			level >= _aDevices[i].min_level ) {
			if ( !(*_aDevices[i].output)(&(_aDevices[i]), level, message) ) {
				_aDevices[i].is_enabled = false;
				log_output(LOG_FATAL, "%s:FAIL!", _aDevices[i].name);
				_aDevices[i].is_enabled = true;
			}
		}
	}
}
