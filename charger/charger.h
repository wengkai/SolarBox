#ifndef _CHARGER_HEAD_
#define _CHARGER_HEAD_

typedef struct {
	double vbattery;
	double isolar;
	double iac;
	double inet;
	double idesktop;
	double iradio1;
	double iradio2;
} Charger;

int charger_init();
int charger_start();
Charger charger_get_instant_data();
Charger charger_get_data();
int charger_is_ac_charging();

#endif
