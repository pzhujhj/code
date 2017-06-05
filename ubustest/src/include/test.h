#ifndef __TEST_H_
#define __TEST_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <libubox/blob.h>
#include <libubox/kvlist.h>


typedef struct{
	char xAccl[12];
	char yAccl[12];
	char zAccl[12];
	uint16_t angle;
	uint8_t threshold;
	int event;
	char result[12];
}SENSOR_MSG;

void add_sensorinfo_to_blob(struct blob_buf *buf);
void sensor_iic_handle();
SENSOR_MSG *get_sensor_value();

#endif
