#ifndef __GPSD_H_
#define __GPSD_H_

//#include <libubox/ustream.h>

#define FREQUENCY 1000
#define TIMER_OP 3000
#define CHECK_GPS_INTERVAL 1
#define GPS_SLAVE_ADDR 0x30
#define I2C_TYPE 1
#define UART_TYPE 0
#define OTHER_TYPE 2
#define NMEA_MAXSAT 12
#define NMEA_SATINPACK 4

typedef struct _GPSLOG {
	FILE *gps_fp;
	char filename[64];
	char backfile[64];
	int filesize;
	int status;
}GPSLOG;

typedef struct _nmeaSATELLITE {
	int id;
	int elv;
	int azimuth;
	int sig;
}nmeaSATELLITE;

typedef struct _nmeaGSV {
	int pack_count; //语句总数
	int pack_index; //当前语句号
	int sat_count; //每一条语句可见卫星数
	nmeaSATELLITE sat_data[35]; //存放每一个卫星信息标识符—仰角—方位角—SNR
}nmeaGSV;

typedef struct {
	char latitude[16]; //纬度
	char longitude[16]; //经度
	char elevation[16]; //海拔
	char latdirection[4]; //纬度方向
	char londirection[4]; //经度方向
	char time[20];
	char angle[24]; //角度
	int satellites_count; //正在使用的卫星数
	char antenna_status[24]; //天线状态
	char speed[24]; //速度 
	char cog[24]; //对地真航向
	char status[2]; //数据有效性
	int sate_gps_id[NMEA_MAXSAT]; //gps正在使用卫星标识符
	int sate_bd_id[NMEA_MAXSAT]; //bd正在使用卫星标识符
	int ave_signal; //平均信号值
	int gp_vis_sate; //可见gp卫星数
	int bd_vis_sate; //可见bd卫星数
	int visible_satellite; //可视卫星总数量
	nmeaGSV nmeagsv;
	char path[64];
} GPS_MSG;

#endif
