#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <syslog.h>
#include <stdarg.h>
#include <uci.h>
#include <math.h>
#include <sys/time.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#if 0
#include <libubox/uloop.h>
#include <libubox/kvlist.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#endif

#include "gpstool.h"

#define _unused __attribute__((unused))


#define LOG_TAG "GPS"
#define GPS_BUFSZ 2048
#define GPS_SV_NR_MAX 32

#define MAX_NMEA_SENTENCE 84
//#define MAX_NMEA_PARAM	20
#define MAX_NMEA_PARAM	32
#define MAX_BUF_SIZE 2048

#define ARRAY_MSG_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define DD(s) ((int)((s)[0]-'0')*10+(int)((s)[1]-'0'))

#define FILE_MAX_SIZE (1024*1024) //1M
#define PI 3.14159265358979323846

enum {
	GPS_SER_CLOSE,
	GPS_SER_OPEN,
	GPS_LOG_OPEN,
	GPS_LOG_CLOSE
};

enum {
	ANTENNA_OK,
	ANTENNA_OPEN,
	ANTENNA_SHORT,
	ANTENNA_UNKNOWN
};

enum {
	GNSS,
	GPS,
	BDS
};

struct gps_buf {
	char *tail;
	char *end;
	char *data;
	char *head;
};

struct gps_satellite {
	int svid;
	int elevation;
	int azimuth;
	int snr;
};

struct nmea_param {
	char *str;
	int num;
} nmea_params[MAX_NMEA_PARAM];

static FILE *gps_fp = NULL;
static int set_time = 0;
static time_t time_set = 0;
static int tz = 0;
static float olat = 0.0;
static float olon = 0.0;
static int gps_ant = ANTENNA_OPEN;
static int gps_mode = GNSS;

static int gps_valid = 0;
static int signal_total[128];
static int sate_gps_use = 0;
static int sate_bd_use = 0;
static int signal_num = 0;
/*
   static int gps_sv_nr = 0;
   static struct gps_satellite gps_svs[GPS_SV_NR_MAX];
   */
static int verbose = 0;
static int use_syslog = 0;

static struct gps_buf *gbuf = NULL;

static int offset = 0;
static int fileTmp = 0;
static int signalFlag = 0;

GPS_MSG gps_msg;

static void glog(int priority, const char *format, ...)
{
	va_list ap;
	FILE *stream;

	if (priority == LOG_DEBUG && verbose < 2)
		return;

	if (priority == LOG_INFO && verbose < 1)
		return;

	va_start(ap, format);
	if (use_syslog)
		vsyslog(priority, format, ap);
	else {
		if (priority == LOG_ERR)
			stream = stderr;
		else
			stream = stdout;

		vfprintf(stream, format, ap);
	}
	va_end(ap);
}

static struct gps_buf *gps_buf_alloc(size_t size)
{
	struct gps_buf *buf = calloc(1, sizeof(*buf) + size);
	if (buf == NULL)
		return NULL;

	buf->head = (char *)buf + sizeof(*buf);
	buf->end  = buf->head + size;
	buf->data = buf->tail = buf->head;

	return buf;
}

static void gps_buf_free(struct gps_buf *buf)
{
	free(buf);
}

/* set system date and time */
static int set_date(struct tm *date)
{
	struct tm now;
	time_t t1, t2;

	t1 = time(NULL);
	if (t1 < 0) {
		glog(LOG_ERR, "Can not get current time");
		return -1;
	}

	memcpy(&now, localtime(&t1), sizeof(now));

	now.tm_sec = date->tm_sec;
	now.tm_min = date->tm_min;
	//now.tm_hour = date->tm_hour + (-timezone) / 3600;
	now.tm_hour = date->tm_hour;
	now.tm_mday = date->tm_mday;
	now.tm_mon = date->tm_mon;
	now.tm_year = date->tm_year;

	t2 = mktime(&now);
	if (t2 < 0) {
		glog(LOG_ERR, "Can not mktime");
		return -1;
	}

	if (tz != timezone) { // in case of timezone changed
		t2 += (-timezone); // add timezone only first time
		tz = -timezone;
	}

	if (t2 - t1 >= 10) { // time difference more than 10 secs
		if (stime(&t2) < 0) {
			glog(LOG_ERR, "Can not set time");
			return -1;
		}
		time_set = t2; // record last time we set
	}

	return 0;
}

static void nmea_rmc_cb(char *msg ,char *buff)
{
	struct tm tm;
//	time_t time;
	char tmp[256];

	snprintf(gps_msg.status, sizeof(gps_msg.status), "%s", nmea_params[2].str);

	//get state work mode
	if (!strcmp("GNRMC", msg))
		gps_mode = GNSS;
	else if (!strcmp("BDRMC", msg))
		gps_mode = BDS;
	else if (!strcmp("GPRMC", msg))
		gps_mode = GPS;

	if (*nmea_params[2].str != 'A') {
		memset(&gps_msg, 0, sizeof(GPS_MSG));
		memcpy(gps_msg.status, "V", 1);
		gps_valid = 0;
		return;
	}

	if (!nmea_params[1].str[0])
		return;

	gps_valid = 1;
	memset(&tm, 0, sizeof(tm));
	tm.tm_isdst = 1;

	tm.tm_hour = DD(nmea_params[1].str);
	tm.tm_min  = DD(nmea_params[1].str + 2);
	tm.tm_sec  = DD(nmea_params[1].str + 4);

	tm.tm_mday = DD(nmea_params[9].str);
	tm.tm_mon  = DD(nmea_params[9].str + 2) - 1;
	tm.tm_year = DD(nmea_params[9].str + 4) + 100;

	strftime(tmp, 256, "%F %T", &tm);
	glog(LOG_INFO, "date: %s UTC\n", tmp);

	if (set_time && time_set == 0)
		set_date(&tm);

	if (!nmea_params[3].str[0] || !nmea_params[5].str[0])
		return;

	char longitude[32], latitude[32];
	double lat, lon;
	double d, m;
//	float lat, lon;
//	float d, m;
	float speed = 0, course = 0;
	int speed_valid = 0;
//	int course_valid = 0;

	if (nmea_params[7].str[0] && strcmp(nmea_params[7].str, "NaN")) {
		speed = atof(nmea_params[7].str); //knots
		speed *= 1.852; // Km/h
		speed_valid = 1;
	}

	if (nmea_params[8].str[0] && strcmp(nmea_params[8].str, "NaN")) {
		course = atof(nmea_params[8].str);
		//course_valid = 1;
	}

	lat = atof(nmea_params[3].str);
	m = 100.0 * modf(lat / 100.0, &d);
	//m = 100.0 * modff(lat / 100.0, &d);
	lat = d + m / 60.0;
	if (*nmea_params[4].str == 'S')
		lat = -lat;

	lon = atof(nmea_params[5].str);
	m = 100.0 * modf(lon / 100.0, &d);
	//m = 100.0 * modff(lon / 100.0, &d);
	lon = d + m / 60.0;
	if (*nmea_params[6].str == 'W')
		lon = -lon;

	snprintf(latitude, sizeof(latitude), "%.6f", lat);
	snprintf(longitude, sizeof(longitude), "%.6f", lon);
	glog(LOG_INFO, "position: %s %s\n", latitude, longitude);

//	if (fabs(olat - lat) >= 1e-6 || fabs(olon - lon) >= 1e-6) {
		snprintf(gps_msg.latitude, sizeof(gps_msg.latitude), "%s", latitude);

		snprintf(gps_msg.latdirection, sizeof(gps_msg.latdirection), "%s", nmea_params[4].str);

		snprintf(gps_msg.longitude, sizeof(gps_msg.longitude), "%s", longitude);

		snprintf(gps_msg.londirection, sizeof(gps_msg.londirection), "%s", nmea_params[6].str);

		olat = lat;
		olon = lon;
//	}

//	time = mktime(&tm);
//	time -= timezone;
//	localtime_r(&time, &tm);

	snprintf(gps_msg.time, sizeof(gps_msg.time), "%s", tmp);

	//对地速度，单位节*1.852=公里
	if (speed_valid) {
		snprintf(gps_msg.speed, sizeof(gps_msg.speed), "%.1f", speed);
	}

	//对地真航向,单位度
//	if (course_valid) {
	snprintf(gps_msg.cog, sizeof(gps_msg.cog), "%.1f", course);
//	}
}

static void nmea_gga_cb(char *msg, char *buff)
{
	int gps_sv_nr = 0;
	float eli;

	if (!gps_valid)
		return;

	//海拔高度
	eli = atof(nmea_params[9].str);
	snprintf(gps_msg.elevation, sizeof(gps_msg.elevation), "%.1f", eli);

	//使用卫星数
	gps_sv_nr = nmea_params[7].num;
	gps_msg.satellites_count = gps_sv_nr;
}

#if 0
	static void
nmea_vtg_cb(void)
{
	char str[64];
	float course = 0.0;
	float speed = 0.0;
	int course_valid = 0;
	int speed_valid  = 0;

	if (!gps_valid)
		return;

	if (nmea_params[1].str[0] && nmea_params[2].str[0] == 'T'
			&& strcmp(nmea_params[1].str, "NaN")) {
		course_valid = 1;
		course = atof(nmea_params[1].str);

		sprintf(str, "%s", "gps.position.cog");
		if (uci_revert_state(ctx, str) < 0)
			return;

		sprintf(str, "gps.position.cog=%.1f", course);
		if (uci_set_state(ctx, str) < 0)
			return;
	}

	if (nmea_params[7].str[0] && nmea_params[8].str[0] == 'K'
			&& strcmp(nmea_params[7].str, "NaN")) {
		speed_valid = 1;
		speed = atof(nmea_params[7].str);

		sprintf(str, "%s", "gps.position.sog");
		if (uci_revert_state(ctx, str) < 0)
			return;

		sprintf(str, "gps.position.sog=%.1f", speed);
		if (uci_set_state(ctx, str) < 0)
			return;
	}

	if (course_valid || speed_valid)
		glog(LOG_INFO, "course: %.1f, speed: %.1f Km/h\n", course, speed);
}
#endif

static void  find_signal_data(int *value, int length, int *total)
{
	int line, i;

	for (i = 0; i < NMEA_MAXSAT; i++) {
		for (line = 0; line < length; line++) {
			if (gps_msg.nmeagsv.sat_data[line].id == value[i]) {
				total[signal_num++] = gps_msg.nmeagsv.sat_data[line].sig;
				break;
			}
		}
	}

	if (signal_num > sizeof(signal_total)/sizeof(int))
		signal_num = 0;
}

static void nmea_gsv_cb(char *msg, char *buff)
{
	int count;
	int ave_signal = 0;
	int sum = 0;
	int isat, isi, nsat;
	int flag = 0;
//	int angle = 0;

//	if (!gps_valid)
//		return;

	//当前可见卫星数
	if ((!strcmp(nmea_params[2].str, "1")) &&  !strcmp("GPGSV", msg))
		gps_msg.gp_vis_sate = nmea_params[3].num;

	if ((!strcmp(nmea_params[2].str, "1")) &&  !strcmp("BDGSV", msg))
		gps_msg.bd_vis_sate = nmea_params[3].num;


	gps_msg.visible_satellite = (gps_msg.gp_vis_sate+gps_msg.bd_vis_sate);

	//处理每一条gsv数据,获取每一个可见卫星的信号值
	gps_msg.nmeagsv.pack_count = nmea_params[1].num;
	gps_msg.nmeagsv.pack_index = nmea_params[2].num;
	gps_msg.nmeagsv.sat_count = nmea_params[3].num;
	nsat = (gps_msg.nmeagsv.pack_index - 1) * NMEA_SATINPACK;
	nsat = (nsat + NMEA_SATINPACK > gps_msg.nmeagsv.sat_count)?(gps_msg.nmeagsv.sat_count - nsat):NMEA_SATINPACK;

	if (gps_msg.satellites_count >= 3) {
		if (!strcmp("GPGSV", msg)) {
			for(isat = 0; isat < nsat; ++isat) {
				isi = (gps_msg.nmeagsv.pack_index - 1) * NMEA_SATINPACK + isat;
				gps_msg.nmeagsv.sat_data[isi].id = nmea_params[isat*NMEA_SATINPACK+4].num;
				gps_msg.nmeagsv.sat_data[isi].elv = nmea_params[isat*NMEA_SATINPACK+5].num;
				gps_msg.nmeagsv.sat_data[isi].azimuth = nmea_params[isat*NMEA_SATINPACK+6].num;
				gps_msg.nmeagsv.sat_data[isi].sig = nmea_params[isat*NMEA_SATINPACK+7].num;

				if (isi == (gps_msg.nmeagsv.sat_count-1))
					flag = 1;
			}

			if (flag == 1) {
				find_signal_data(gps_msg.sate_gps_id, gps_msg.nmeagsv.sat_count, signal_total);
				memset(&gps_msg.nmeagsv, 0, sizeof(gps_msg.nmeagsv));
			}
		}else if (!strcmp("BDGSV", msg)){
			for(isat = 0; isat < nsat; ++isat) {
				isi = (gps_msg.nmeagsv.pack_index - 1) * NMEA_SATINPACK + isat;
				gps_msg.nmeagsv.sat_data[isi].id = nmea_params[isat*NMEA_SATINPACK+4].num;
				gps_msg.nmeagsv.sat_data[isi].elv = nmea_params[isat*NMEA_SATINPACK+5].num;
				gps_msg.nmeagsv.sat_data[isi].azimuth = nmea_params[isat*NMEA_SATINPACK+6].num;
				gps_msg.nmeagsv.sat_data[isi].sig = nmea_params[isat*NMEA_SATINPACK+7].num;

				if (isi == (gps_msg.nmeagsv.sat_count-1))
					flag = 1;
			}

			if (flag == 1) {
				find_signal_data(gps_msg.sate_bd_id, gps_msg.nmeagsv.sat_count, signal_total);
				memset(&gps_msg.nmeagsv, 0, sizeof(gps_msg.nmeagsv));
			}
		}

		if (signal_num > gps_msg.satellites_count)
			signal_num = gps_msg.satellites_count;

		if (signal_num == gps_msg.satellites_count) {
			for (count = 0; count < signal_num; count++) {
				sum += signal_total[count];
			}

			if (count == signal_num) {
				ave_signal = sum/signal_num;
				signal_num = 0;

				memset(gps_msg.sate_gps_id, 0, sizeof(gps_msg.sate_gps_id));
				memset(gps_msg.sate_bd_id, 0, sizeof(gps_msg.sate_bd_id));
				memset(signal_total, 0, sizeof(signal_total));
			}

			gps_msg.ave_signal = ave_signal;
			glog(LOG_INFO, "time=%s\n", gps_msg.time);
			glog(LOG_INFO, "ave_signal=%d\n", ave_signal);
		}
	}else {
		memset(&gps_msg.nmeagsv, 0, sizeof(gps_msg.nmeagsv));
	}

#if 0
	//真方位角
	angle = nmea_params[6].num;
	if ((!strcmp(nmea_params[2].str, "1")) &&  !strcmp("GPGSV", msg)) {
		sprintf(str, "%s", "gps.position.angle");
		if (uci_revert_state(ctx, str) < 0)
			return;

		sprintf(str, "gps.position.angle=%d", angle);
		if (uci_set_state(ctx, str) < 0)
			return;

		memset(str, 0, sizeof(str));

		sprintf(str, "%d", angle);
		memcpy(gps_msg.angle, str, strlen(str));
	}
#endif
}

static void nmea_txt_cb(char *msg, char *buff)
{
	char str[64] = {0};

	memset(str, 0, sizeof(str));
	sprintf(str, "%s", "gps.position.antenna");

	if (!strcmp("ANTENNA OPEN", nmea_params[4].str)) {
		sprintf(str, "gps.position.antenna=%s", "1");
		gps_ant = ANTENNA_OPEN;
	}else if (!strcmp("ANTENNA SHORT", nmea_params[4].str)) {
		sprintf(str, "gps.position.antenna=%s", "2");
		gps_ant = ANTENNA_SHORT;
	}
	else if (!strcmp("ANTENNA OK", nmea_params[4].str)) {
		sprintf(str, "gps.position.antenna=%s", "0");
		gps_ant = ANTENNA_OK;
	}
	else if (!strcmp("ANTENNA UNKNOWN", nmea_params[4].str)) {
		sprintf(str, "gps.position.antenna=%s", "3");
		gps_ant = ANTENNA_UNKNOWN;
	}

}

static void nmea_gsa_cb(char *msg, char *buff)
{
	int line;
	char *sysid = NULL;

	//模式1:定位不可用
	if ((nmea_params[2].num == 1) || (gps_msg.satellites_count == 0))
		return;

	//判断系统标识符
	sysid = strrchr(buff, ',');

	//收集正在使用卫星的标志号,根据gsa数据报文中使用卫星的标志号最多为12个
	for(line = 3; line < NMEA_MAXSAT+3; line++) {
		if (nmea_params[line].num == 0)
			break;
		else {
			if (sysid[1] == '1')   //gps gsa
				gps_msg.sate_gps_id[sate_gps_use++] = nmea_params[line].num;
			else if (sysid[1] == '4') //bd gsa
				gps_msg.sate_bd_id[sate_bd_use++] = nmea_params[line].num;
		}
	}

	if ((sate_gps_use+sate_bd_use) >= gps_msg.satellites_count) {
		sate_gps_use = 0;
		sate_bd_use = 0;
	}
}

static struct nmea_msg {
	char *msg;
	int cnt;
	void (*handler) (char *mssage, char *buff);
} nmea_msgs[] = {
	{
		.msg = "RMC",
		.cnt = 14,
		.handler = nmea_rmc_cb,
		/*	}, {
			.msg = "VTG",
			.cnt = 9,
			.handler = nmea_vtg_cb,
			*/
	}, {
		.msg = "GGA",
		.cnt = 15,
		.handler = nmea_gga_cb,
	}, {
		.msg = "GSV",
		.cnt = 21,
		.handler = nmea_gsv_cb,
	}, {
		.msg = "TXT",
		.cnt = 5,
		.handler = nmea_txt_cb,
	}, {
		.msg = "GSA",
		.cnt = 19,
		.handler = nmea_gsa_cb,
	}
};

static int
nmea_verify_checksum(char *s)
{
	char *csum = strrchr(s, '*');
	int isum, c = 0;

	if (!csum)
		return -1;

	*csum = '\0';
	csum++;
	isum = strtol(csum, NULL, 16);

	while(*s)
		c ^= *s++;

	if (isum != c)
		return -1;

	return 0;
}

static int
nmea_tokenize(char *msg)
{
	int cnt = 0;
	char *tok = msg;
	char *ptr = tok;

	while (*ptr != '\0' && cnt < MAX_NMEA_PARAM) {
		if (*ptr == ',') {
			*ptr = '\0';
			nmea_params[cnt].str = tok;
			nmea_params[cnt].num = atoi(tok);
			tok = ptr + 1;
			cnt++;
		}
		ptr++;
	}

	if (*ptr == '\0' && tok <= ptr) {
		nmea_params[cnt].str = tok;
		nmea_params[cnt].num = atoi(tok);
		cnt++;
	}

	return cnt;
}

static void
nmea_process(char *msg)
{
	char *csum;
	char buf[120] = {0};
	int cnt, i;

	memset(buf, 0, sizeof(buf));
	memcpy(buf, msg, strlen(msg));

	if (strncmp(msg, "$GP", 3) && strncmp(msg, "$GN", 3) && strncmp(msg, "$BD", 3))
		return;

	msg++;
	csum = strrchr(msg, '*');
	if (!csum)
		return;

	if (nmea_verify_checksum(msg)) {
		glog(LOG_DEBUG, "nmea message has invlid checksum\n");
		return;
	}

	cnt = nmea_tokenize(&msg[2]);
	if (cnt < 0) {
		glog(LOG_NOTICE, "failed to tokenize %s\n", msg);
		return;
	}

	for (i = 0; i < ARRAY_MSG_SIZE(nmea_msgs); i++) {
		if (strcmp(nmea_params[0].str, nmea_msgs[i].msg))
			continue;
		if (nmea_msgs[i].cnt >= cnt) {
			nmea_msgs[i].handler(msg, buf);
		}
		else {
			glog(LOG_NOTICE, "%s datagram has wrong parameter count got %d but expected %d\n",
					nmea_msgs[i].msg, cnt, nmea_msgs[i].cnt);
		}
		return;
	}
}

static int
nmea_consume(char *s, const char *end)
{
	char *p = s;

	//while (p < end && *p != '\n')
	while (p < end && *p != '\r')
		p++;

	//printf("--1---p=%p s=%p  end=%p\n", p, s, end);
	//printf("----1---%d %d\n", strlen(p), strlen(s));
	if (p == end) {
		if (p - s >= MAX_NMEA_SENTENCE)
			return p - s;
		return -1;
	}

	*p++ = '\0';
	//printf("--2---p=%p s=%p  end=%p\n", p, s, end);
	//printf("----2---%d %d\n", strlen(p), strlen(s));
	if (p < end && *p == '\n') {
		*p++ = '\0';
		//printf("----3---%d %d\n", strlen(p), strlen(s));
	}

	//printf("--3---p=%p s=%p  end=%p\n", p, s, end);
	while (p < end && (*p == '\r' || *p == '\n')) {
		*p++ = '\0';
	}

	//printf("--4---p=%p s=%p  end=%p\n", p, s, end);
	if ((p - s > 2) && (p - s <= MAX_NMEA_SENTENCE)) {
		//glog(LOG_DEBUG, "%s\n", s);
		nmea_process(s);
	}

	return p - s;
}

static void nmea_msg_cb(struct gps_buf *buf)
{
	int ret;

	while (buf->data < buf->tail) {
		ret = nmea_consume(buf->data, buf->tail);
		if (ret < 0 || ret > 84) {
			//if (buf->tail == buf->end)
			memcpy(gps_msg.status, "V", 1);
			buf->data = buf->tail = buf->head;
			break;
		}

		buf->data += ret;
	}

	if (buf->data == buf->tail) {
		buf->data = buf->tail = buf->head;
	}
}

static int get_file_data(char *msg)
{
	char linebuf[128] = {0};
	char buf[1024] = {0};
	int ret = -1;
	char ch;
	int count = 0;

	memset(linebuf, 0, 84);
	memset(buf, 0, 1024);

	while (!feof(gps_fp)) {
		if((ch=fgetc(gps_fp)) == EOF) {
			return 0;
		}
		fgets(linebuf, 84, gps_fp);
		if(!strncmp(linebuf+2, "TXT", 3))
			break;
	}

	offset = ftell(gps_fp);
	fseek(gps_fp, fileTmp, SEEK_SET);
	count = (offset-fileTmp)>1024?1024:(offset-fileTmp);
	ret = fread(buf, count, 1, gps_fp);
	if (ret == 0) {
		signalFlag = 1;
		printf("fread error!\n");
	}
	fileTmp = offset;
	fseek(gps_fp, fileTmp, SEEK_SET);

	memcpy(msg, buf, count);

	return count;
}

static int handle_nmea_msg(struct gps_buf *buf)
{
	int ret;
	size_t count;
	char value[1024] = {0}; 

	for (;;) {
		count = buf->end - buf->tail;
		if (count == 0) {
			nmea_msg_cb(buf);
			continue;
		}
		do {
			memset(value, 0, 1024);
			ret = get_file_data(value);	
			memcpy(buf->tail, value, ret);
			//buf->tail[ret] = '\0';
		} while (ret < 0 && errno == EINTR);

		glog(LOG_DEBUG, "%s\n",buf->tail);

		if (ret <= 0)
			return ret;

		buf->tail += ret;
		nmea_msg_cb(buf);

		if (ret < count)
			break;
	}

	return ret;
}

static void handle_INT(int signum)
{
	glog(LOG_NOTICE, "Terminated by signal %d\n", signum);

	if (gps_fp) {
		fclose(gps_fp);
		gps_fp = NULL;
	}

	if (gbuf)
		gps_buf_free(gbuf);

	if (use_syslog)
		closelog();

	exit(1);
}

static void setup_signals()
{
	struct sigaction sa;

	sa.sa_handler = handle_INT;
	sa.sa_flags = 0;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

static void daemonize()
{
	int i;

	daemon(0, 0);

	for (i = 0; i < getdtablesize(); i++) {
		if (close(i) < 0)
			break;
	}
}

static double toradians(double degree)
{
	double r = degree*PI/180;
	return r;
}

double getdistance(double lat1, double lon1, double lat2, double lon2)
{
	double a = 6378137, b = 6356752.314245, f = 1/298.257223563;
	double L = toradians(lon2 - lon1);

	double U1 = atan((1 - f)*tan(toradians(lat1)));
	double U2 = atan((1 - f)*tan(toradians(lat2)));
	double sinU1 = sin(U1), cosU1 = cos(U1);
	double sinU2 = sin(U2), cosU2 = cos(U2);
	double cosSqAlpha;
	double sinSigma;
	double cos2SigmaM;
	double cosSigma;
	double sigma;

	double lambda = L, lambdaP, iterLimit = 100;
	do
	{
		double sinLambda = sin(lambda), cosLambda = cos(lambda);
		sinSigma = sqrt((cosU2*sinLambda)*(cosU2*sinLambda)+ (cosU1*sinU2 - sinU1*cosU2*cosLambda)*(cosU1*sinU2 - sinU1*cosU2*cosLambda));

		if (sinSigma == 0)
		{
			return 0;
		}

		cosSigma = sinU1*sinU2 + cosU1*cosU2*cosLambda;
		sigma = atan2(sinSigma, cosSigma);
		double sinAlpha = cosU1*cosU2*sinLambda/sinSigma;
		cosSqAlpha = 1 - sinAlpha*sinAlpha;
		cos2SigmaM = cosSigma - 2*sinU1*sinU2/cosSqAlpha;

		double C = f/16*cosSqAlpha*(4 + f*(4 - 3*cosSqAlpha));
		lambdaP = lambda;
		lambda = L + (1 - C)*f*sinAlpha*(sigma + C*sinSigma*(cos2SigmaM + C*cosSigma*(-1 + 2*cos2SigmaM*cos2SigmaM)));
	} while (abs(lambda - lambdaP) > 1e-12 && --iterLimit > 0);

	if (iterLimit == 0)
	{
		return 0;
	}

	double uSq = cosSqAlpha*(a*a - b*b)/(b*b);
	double A = 1 + uSq/16384*(4096 + uSq*(-768 + uSq*(320 - 175*uSq)));
	double B = uSq/1024*(256 + uSq*(-128 + uSq*(74 - 47*uSq)));
	double deltaSigma = B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));

	double s = b*A*(sigma - deltaSigma);

	return s;
}

void parseGpsInfo()
{
	int fd = -1;
	int wbyte = 0;
	int num = 0;
	char linebuf[1024] = {0};

	fd = open("/home/jihj/test/gpstool/gps.txt", O_RDWR | O_APPEND |O_CREAT, 0777);
	if (fd < 0) {
		printf("open error:%s\n", strerror(errno));
		exit(1);
	}

	memset(linebuf, 0, sizeof(linebuf));
	num = sprintf(linebuf, "时间:%s 纬度:%s 纬度方向:%s 经度:%s 经度方向:%s gps状态:%s 正在使用卫星数:%d 可见卫星数:%d 速度:%s 对地真航向:%s 平均信号值:%d 天线状态:%d\r\n", gps_msg.time,gps_msg.latitude, gps_msg.latdirection, gps_msg.longitude, gps_msg.londirection, gps_msg.status, gps_msg.satellites_count, gps_msg.visible_satellite, gps_msg.speed,gps_msg.cog, gps_msg.ave_signal, gps_ant);

	wbyte = write(fd, linebuf, num);	
	if (wbyte < 0) 
		printf("write error:%s\n", strerror(errno));

	close(fd);
}

void sig_prof(int signo)
{
	handle_nmea_msg(gbuf);
	parseGpsInfo();
}

void init_sigaction(void)
{
	struct sigaction act;

	act.sa_handler=sig_prof;
	act.sa_flags=0;
	sigemptyset(&act.sa_mask);

	sigaction(SIGALRM,&act,NULL); //SIGALRM 用于定时器的信号
}

/*定时器的使用*/
void set_timer(void)
{
	struct itimerval itv;
	itv.it_value.tv_sec=1;//秒  1s后第一次启动定时器
	itv.it_value.tv_usec=0;//微秒

	itv.it_interval.tv_sec = 1; //启动后每隔一秒重新执行
    itv.it_interval.tv_usec = 0;
	
	setitimer(ITIMER_REAL,&itv,NULL);
}

static void usage(void)
{
	fprintf(stderr,
			"option: \n"
			"       -d file path\n"
			"       -v show more debug messages (-vv for even more)\n"
			"       -h show this usage\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int opt;
	int daemon = 0;
	const char *device = NULL;
	int filesize = 0;

	while ((opt = getopt(argc, argv, "d:c:h:v")) != -1) {
		switch (opt) {
			case 'd':
				device = optarg;
				break;
			case 's':
				set_time = 1;
				break;
			case 'v':
				verbose++;
				break;
			case 'D':
				daemon = 1;
				break;
			case 'h':
				usage();
				break;
			default:
				usage();
				break;
		}
	}

	if (device == NULL)
		usage();

	if (daemon) {
		daemonize();
		openlog(LOG_TAG, LOG_PID, LOG_DAEMON);
		use_syslog = 1;
	}

	setup_signals();

	gbuf = gps_buf_alloc(GPS_BUFSZ);
	if (gbuf == NULL) {
		glog(LOG_ERR, "Out of memory");
		return -1;
	}

	gps_fp = fopen(device, "r");	
	if (gps_fp == NULL) {
		glog(LOG_DEBUG, "fopen device fail!\n");
		exit(1);
	}
	
	//计算文件大小
	fseek(gps_fp, 0, SEEK_END);
	filesize = ftell(gps_fp);

	//计算文件大小后，需要重新指向文件的开头
	fseek(gps_fp, 0, SEEK_SET);

	//timer
	init_sigaction();
	set_timer();

	while(fileTmp < filesize);

	fclose(gps_fp);
	gps_fp = NULL;

	gps_buf_free(gbuf);

	if (use_syslog)
		closelog();

	return 0;
}
