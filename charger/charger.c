#include "charger.h"
#include "../log/log.h"
#include "../tlc1543/tlc1543.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

static TLC1543 __tlc1543;
static pthread_t __charger_thread_id;
static pthread_mutex_t __filter_array_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t __is_ac_charging_lock = PTHREAD_MUTEX_INITIALIZER;

static void *_charger_thread(void *arg);

// static const int WINDOW_SIZE = 16;
#define WINDOW_SIZE 12
static const int SAMPLE_INTERVAL = 5; //	in seconds

static Charger __filter_array[WINDOW_SIZE] = {{0}};
static int __current_position = 0;
static int __is_ac_charging = 0;

static const double VREF = 5010.0;
static const int RESULOTION = 1024;

#define voltage(d) 			((d)*VREF/RESULOTION)
//#define current(d, zero) 	((-1)*(voltage(d)-(zero))/100)
#define current(d, zero) 	((voltage(d))>(zero)?0:(-1)*(voltage(d)-(zero))/100)

static void _check_battery(void);

int charger_init()
{
	int ret = 0;
	ret = pthread_mutex_init(&__filter_array_lock, NULL);
	if ( ret == 0 ) {
		ret = pthread_mutex_init(&__is_ac_charging_lock, NULL);
		if ( ret == 0 ) {
			ret = tlc1543_init(&__tlc1543, 0, 4000000, 0);
		}
	}
	return ret;
}

int charger_start()
{
	int ret = 0;
	int err = pthread_create(&__charger_thread_id, NULL, _charger_thread, NULL);
	if ( err ) 
    { 
        log_output(LOG_FATAL, "can't create charger thread:%s", strerror(err)); 
        ret = -1;
    }
    return ret;
}

Charger charger_get_instant_data()
{
	Charger ret = {0};
	pthread_mutex_lock(&__filter_array_lock);
	int pos = __current_position -1;
	if ( pos <0 )
		pos = WINDOW_SIZE -1;
	ret.vbattery 	= __filter_array[pos].vbattery;
	ret.isolar		= __filter_array[pos].isolar;
	ret.iac			= __filter_array[pos].iac;
	ret.inet 		= __filter_array[pos].inet;
	ret.idesktop	= __filter_array[pos].idesktop;
	ret.iradio1 		= __filter_array[pos].iradio1;
	ret.iradio2		= __filter_array[pos].iradio2;
	pthread_mutex_unlock(&__filter_array_lock);
	// log_output(LOG_DETAIL, "pos=%d", pos);
	return ret;
}

Charger charger_get_data()
{
	Charger ret = {0};
	pthread_mutex_lock(&__filter_array_lock);
	for ( int i=0; i<WINDOW_SIZE; i++ ) {
		ret.vbattery 	+= __filter_array[i].vbattery;
		ret.isolar		+= __filter_array[i].isolar;
		ret.iac			+= __filter_array[i].iac;
		ret.inet 		+= __filter_array[i].inet;
		ret.idesktop	+= __filter_array[i].idesktop;
		ret.iradio1 		+= __filter_array[i].iradio1;
		ret.iradio2		+= __filter_array[i].iradio2;
	}
	pthread_mutex_unlock(&__filter_array_lock);
	ret.vbattery 	/= WINDOW_SIZE;
	ret.isolar 		/= WINDOW_SIZE;
	ret.iac 		/= WINDOW_SIZE;
	ret.inet 		/= WINDOW_SIZE;
	ret.idesktop 	/= WINDOW_SIZE;
	ret.iradio1 		/= WINDOW_SIZE;
	ret.iradio2		/= WINDOW_SIZE;
	return ret;
}

int charger_is_ac_charging()
{
	int ret = 0;
	pthread_mutex_lock(&__is_ac_charging_lock);
	ret = __is_ac_charging;
	pthread_mutex_unlock(&__is_ac_charging_lock);
	return ret;
}

//--------------------------------------------------------------

static void *_charger_thread(void *arg)
{
	int number_of_samples = 0;
	int values[16] = {0};
	while ( 1 ) {
		pthread_mutex_lock(&__filter_array_lock);
		tlc1543_read_all(&__tlc1543, values);		
		__filter_array[__current_position].vbattery = voltage(values[0])*3.175/1000;
		__filter_array[__current_position].isolar 	= current(values[1],2481);
		__filter_array[__current_position].iac 		= current(values[3],2475);
		__filter_array[__current_position].inet 	= current(values[8],2475);
		__filter_array[__current_position].idesktop = current(values[2],2475);
		__filter_array[__current_position].iradio1 	= current(values[4],2471);
		__filter_array[__current_position].iradio2	= current(values[6],2472);	
		// log_output(LOG_DETAIL, " %4.2lf %4.2lf %4.2lf %4.2lf %4.2lf %4.2lf ",
		// 	filter_array[current_position].vbattery,
		// 	filter_array[current_position].isolar,
		// 	filter_array[current_position].iac,
		// 	filter_array[current_position].inet,
		// 	filter_array[current_position].idesktop,
		// 	filter_array[current_position].iradio);
		__current_position = (__current_position+1) % WINDOW_SIZE;
		pthread_mutex_unlock(&__filter_array_lock);
		if ( number_of_samples++ >= WINDOW_SIZE ) {
			_check_battery();
			number_of_samples = 0;
		}
		sleep(SAMPLE_INTERVAL);
	}
	return 0;
}

static void _check_battery(void)
{
	time_t now = time(0);
	struct tm* ptmnow = localtime(&now);
	int is_saving = ptmnow->tm_hour >= 22 || ptmnow->tm_hour <=8;	 
	Charger average = charger_get_data();
	log_output(LOG_DETAIL, "Check battery, v=%4.2lf", average.vbattery);
	int is_charging = 0;
	pthread_mutex_lock(&__is_ac_charging_lock);
	is_charging = __is_ac_charging;
	pthread_mutex_unlock(&__is_ac_charging_lock);
	if ( is_charging ) {
		if ( !is_saving && average.vbattery >= 13.4 ) {
			//	stop charging
			pthread_mutex_lock(&__is_ac_charging_lock);
			__is_ac_charging = 0;
			pthread_mutex_unlock(&__is_ac_charging_lock);
			log_output(LOG_INFO, "Stop charging.");
		}
	} else {	//	not charging
		if ( (is_saving ==1 && average.vbattery < 13.3) || 
			 (is_saving ==0 && average.vbattery < 12.9) )  {
			pthread_mutex_lock(&__is_ac_charging_lock);
			__is_ac_charging = 1;
			pthread_mutex_unlock(&__is_ac_charging_lock);
			log_output(LOG_INFO, "Start charging.");
		}
	}
}
