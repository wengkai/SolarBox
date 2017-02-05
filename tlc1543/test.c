#include "tlc1543.h"
#include <stdio.h>

int main(void)
{
	TLC1543 tlc1543;
	tlc1543_init(&tlc1543, 0, 4000000, 0);
	int values[16] = {0};
	tlc1543_read_all(&tlc1543, values);
	int i;
	for ( i=0; i<14; i++ ) {
		printf("%02d:%04d:%3.1fV\n", i, values[i], values[i]/1024.0*5.01);
	}
	tlc1543_close(&tlc1543);
	return 0;
}
