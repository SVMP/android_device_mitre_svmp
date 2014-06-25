/*
Copyright 2013 The MITRE Corporation, All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this work except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
* Copyright (C) 2008 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <hardware/hardware.h>
#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>
#include <cutils/sockets.h>

#define LOG_TAG "SVMP"
#include <cutils/log.h>

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

/*****************************************************************************/
#define SVMPDEBUG 0

struct svmp_sensor_event_t {
	int type;
	int accuracy;
	long timestamp;
	float value[3];
};

#define SENSOR_HANDLE_ACCELEROMETER       0
#define SENSOR_HANDLE_MAGNETIC_FIELD      1
#define SENSOR_HANDLE_ORIENTATION         2
#define SENSOR_HANDLE_GYROSCOPE           3
#define SENSOR_HANDLE_LIGHT               4
#define SENSOR_HANDLE_PRESSURE            5
//#define SENSOR_HANDLE_TEMPERATURE         5
#define SENSOR_HANDLE_PROXIMITY           6
#define SENSOR_HANDLE_GRAVITY             7
#define SENSOR_HANDLE_LINEAR_ACCELERATION 8
#define SENSOR_HANDLE_ROTATION_VECTOR     9
//#define SENSOR_HANDLE_RELATIVE_HUMIDITY   7
//#define SENSOR_HANDLE_AMBIENT_TEMPERATURE 8

#define HANDLES_MIN 0
#define HANDLES_MAX 9

#define SENSOR_MAX_ACCELEROMETER       20000.0f
#define SENSOR_MAX_MAGNETIC_FIELD      20000.0f
#define SENSOR_MAX_ORIENTATION         20000.0f
#define SENSOR_MAX_GYROSCOPE           20000.0f
#define SENSOR_MAX_LIGHT               20000.0f
#define SENSOR_MAX_PRESSURE            20000.0f
#define SENSOR_MAX_TEMPERATURE         250.0f
#define SENSOR_MAX_PROXIMITY           100.0f
#define SENSOR_MAX_GRAVITY             20000.0f
#define SENSOR_MAX_LINEAR_ACCELERATION 20000.0f
#define SENSOR_MAX_ROTATION_VECTOR     20000.0f
#define SENSOR_MAX_RELATIVE_HUMIDITY   20000.0f
#define SENSOR_MAX_AMBIENT_TEMPERATURE 250.0f

#define SENSOR_RES_ACCELEROMETER       0.01f
#define SENSOR_RES_MAGNETIC_FIELD      0.01f
#define SENSOR_RES_ORIENTATION         0.01f
#define SENSOR_RES_GYROSCOPE           0.01f
#define SENSOR_RES_LIGHT               1.0f
#define SENSOR_RES_PRESSURE            0.01f
#define SENSOR_RES_TEMPERATURE         0.01f
#define SENSOR_RES_PROXIMITY           0.1f
#define SENSOR_RES_GRAVITY             0.01f
#define SENSOR_RES_LINEAR_ACCELERATION 0.01f
#define SENSOR_RES_ROTATION_VECTOR     0.01f
#define SENSOR_RES_RELATIVE_HUMIDITY   0.1f
#define SENSOR_RES_AMBIENT_TEMPERATURE 0.01f

/*****************************************************************************/


  /////////////////////////
 // All Per-Sensor Data //
/////////////////////////

struct sensors_event_t events[] = {
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_ACCELEROMETER, SENSOR_TYPE_ACCELEROMETER, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_MAGNETIC_FIELD, SENSOR_TYPE_MAGNETIC_FIELD, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_ORIENTATION, SENSOR_TYPE_ORIENTATION, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_GYROSCOPE, SENSOR_TYPE_GYROSCOPE, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_LIGHT, SENSOR_TYPE_LIGHT, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_PRESSURE, SENSOR_TYPE_PRESSURE, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
//	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_TEMPERATURE, SENSOR_TYPE_TEMPERATURE, 
//			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_PROXIMITY, SENSOR_TYPE_PROXIMITY, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_GRAVITY, SENSOR_TYPE_GRAVITY, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_LINEAR_ACCELERATION, SENSOR_TYPE_LINEAR_ACCELERATION, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_ROTATION_VECTOR, SENSOR_TYPE_ROTATION_VECTOR, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
//	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_RELATIVE_HUMIDITY, SENSOR_TYPE_RELATIVE_HUMIDITY, 
//			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
//	{	sizeof(struct sensors_event_t), SENSOR_HANDLE_AMBIENT_TEMPERATURE, SENSOR_TYPE_AMBIENT_TEMPERATURE, 
//			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	}
};

// last update time of each device
struct timeval lastPoll[] = { {0,0}, {0,0}, {0,0}, {0,0},
                              {0,0}, {0,0}, {0,0}, {0,0},
                              {0,0}, {0,0}, {0,0}, {0,0},
                              {0,0}, {0,0}, {0,0}, {0,0},
                            };

pthread_t daemon_thread;
static const struct sensor_t sSensorList[] = {

    { "SVMP Remote Accelerometer",            // name
      "SVMP",                                 // vendor
      1,                                      // version
      SENSOR_HANDLE_ACCELEROMETER,            // handle
      SENSOR_TYPE_ACCELEROMETER,              // type
      SENSOR_MAX_ACCELEROMETER,               // maxRange
      SENSOR_RES_ACCELEROMETER,               // resolution
      0.25f,                                  // power
      0,                                      // minDelay
      0,                                      // fifoReservedEventCount
      0,                                      // fifoMaxEventCount
      {}},                                    // reserved

    { "SVMP Remote Magnetic Field Sensor",    // name
      "SVMP",                                 // vendor
      1,                                      // version
      SENSOR_HANDLE_MAGNETIC_FIELD,           // handle
      SENSOR_TYPE_MAGNETIC_FIELD,             // type
      SENSOR_MAX_MAGNETIC_FIELD,              // maxRange
      SENSOR_RES_MAGNETIC_FIELD,              // resolution
      0.25f,                                  // power
      0,                                      // minDelay 
      0,                                      // fifoReservedEventCount
      0,                                      // fifoMaxEventCount
      {}},                                    // reserved

    { "SVMP Remote Orientation Sensor",       // name
      "SVMP",                                 // vendor
      1,                                      // version
      SENSOR_HANDLE_ORIENTATION,              // handle
      SENSOR_TYPE_ORIENTATION,                // type
      SENSOR_MAX_ORIENTATION,                 // maxRange
      SENSOR_RES_ORIENTATION,                 // resolution
      0.25f,                                  // power
      0,                                      // minDelay 
      0,                                      // fifoReservedEventCount
      0,                                      // fifoMaxEventCount
      {}},                                    // reserved

    { "SVMP Remote Gyroscope",                // name
      "SVMP",                                 // vendor
      1,                                      // version
      SENSOR_HANDLE_GYROSCOPE,                // handle
      SENSOR_TYPE_GYROSCOPE,                  // type
      SENSOR_MAX_GYROSCOPE,                   // maxRange
      SENSOR_RES_GYROSCOPE,                   // resolution
      0.25f,                                  // power
      0,                                      // minDelay 
      0,                                      // fifoReservedEventCount
      0,                                      // fifoMaxEventCount
      {}},                                    // reserved

    { "SVMP Remote Light Sensor",             // name
      "SVMP",                                 // vendor
      1,                                      // version
      SENSOR_HANDLE_LIGHT,                    // handle
      SENSOR_TYPE_LIGHT,                      // type
      SENSOR_MAX_LIGHT,                       // maxRange
      SENSOR_RES_LIGHT,                       // resolution
      0.25f,                                  // power
      0,                                      // minDelay 
      0,                                      // fifoReservedEventCount
      0,                                      // fifoMaxEventCount
      {}},                                    // reserved

    { "SVMP Remote Pressure Sensor",          // name
      "SVMP",                                 // vendor
      1,                                      // version
      SENSOR_HANDLE_PRESSURE,                 // handle
      SENSOR_TYPE_PRESSURE,                   // type
      SENSOR_MAX_PRESSURE,                    // maxRange
      SENSOR_RES_PRESSURE,                    // resolution
      0.25f,                                  // power
      0,                                      // minDelay 
      0,                                      // fifoReservedEventCount
      0,                                      // fifoMaxEventCount
      {}},                                    // reserved

//    { "SVMP Remote Temperature Sensor",       // name
//      "SVMP",                                 // vendor
//      1,                                      // version
//      SENSOR_HANDLE_TEMPERATURE,              // handle
//      SENSOR_TYPE_TEMPERATURE,                // type
//      SENSOR_MAX_TEMPERATURE,                 // maxRange
//      SENSOR_RES_TEMPERATURE,                 // resolution
//      0.25f,                                  // power
//      0,                                      // minDelay 
//      0,                                      // fifoReservedEventCount
//      0,                                      // fifoMaxEventCount
//      {}},                                    // reserved

    { "SVMP Remote Proximity Sensor",         // name
      "SVMP",                                 // vendor
      1,                                      // version
      SENSOR_HANDLE_PROXIMITY,                // handle
      SENSOR_TYPE_PROXIMITY,                  // type
      SENSOR_MAX_PROXIMITY,                   // maxRange
      SENSOR_RES_PROXIMITY,                   // resolution
      0.25f,                                  // power
      0,                                      // minDelay 
      0,                                      // fifoReservedEventCount
      0,                                      // fifoMaxEventCount
      {}},                                    // reserved

    { "SVMP Remote Gravity Sensor",           // name
      "SVMP",                                 // vendor
      1,                                      // version
      SENSOR_HANDLE_GRAVITY,                  // handle
      SENSOR_TYPE_GRAVITY,                    // type
      SENSOR_MAX_GRAVITY,                     // maxRange
      SENSOR_RES_GRAVITY,                     // resolution
      0.25f,                                  // power
      0,                                      // minDelay 
      0,                                      // fifoReservedEventCount
      0,                                      // fifoMaxEventCount
      {}},                                    // reserved

    { "SVMP Remote Linear Acceleration Sensor", // name
      "SVMP",                                   // vendor
      1,                                        // version
      SENSOR_HANDLE_LINEAR_ACCELERATION,        // handle
      SENSOR_TYPE_LINEAR_ACCELERATION,          // type
      SENSOR_MAX_LINEAR_ACCELERATION,           // maxRange
      SENSOR_RES_LINEAR_ACCELERATION,           // resolution
      0.25f,                                    // power
      0,                                        // minDelay 
      0,                                        // fifoReservedEventCount
      0,                                        // fifoMaxEventCount
      {}},                                      // reserved

    { "SVMP Remote Rotation Vector Sensor",   // name
      "SVMP",                                 // vendor
      1,                                      // version
      SENSOR_HANDLE_ROTATION_VECTOR,          // handle
      SENSOR_TYPE_ROTATION_VECTOR,            // type
      SENSOR_MAX_ROTATION_VECTOR,             // maxRange
      SENSOR_RES_ROTATION_VECTOR,             // resolution
      0.25f,                                  // power
      0,                                      // minDelay 
      0,                                      // fifoReservedEventCount
      0,                                      // fifoMaxEventCount
      {}},                                    // reserved

//    { "SVMP Remote Relative Humidity Sensor", // name
//      "SVMP",                                 // vendor
//      1,                                      // version
//      SENSOR_HANDLE_RELATIVE_HUMIDITY,        // handle
//      SENSOR_TYPE_RELATIVE_HUMIDITY,          // type
//      SENSOR_MAX_RELATIVE_HUMIDITY,           // maxRange
//      SENSOR_RES_RELATIVE_HUMIDITY,           // resolution
//      0.25f,                                  // power
//      0,                                      // minDelay 
//      0,                                      // fifoReservedEventCount
//      0,                                      // fifoMaxEventCount
//      {}},                                    // reserved

//    { "SVMP Remote Ambient Temperature Sensor", // name
//      "SVMP",                                   // vendor
//      1,                                        // version
//      SENSOR_HANDLE_AMBIENT_TEMPERATURE,        // handle
//      SENSOR_TYPE_AMBIENT_TEMPERATURE,          // type
//      SENSOR_MAX_AMBIENT_TEMPERATURE,           // maxRange
//      SENSOR_RES_AMBIENT_TEMPERATURE,           // resolution
//      0.25f,                                    // power
//      0,                                        // minDelay 
//      0,                                        // fifoReservedEventCount
//      0,                                        // fifoMaxEventCount
//      {}},                                      // reserved

};

/////////////////////

extern "C" {

pthread_mutex_t newpacket_mutex;
pthread_cond_t newpacket_cv;
int cnt=0;


/* Ename SVMPDEBUG to spit out debug to /data/svmpdebug.txt */
#if SVMPDEBUG == 1
 FILE *dfp=fopen ("/data/svmpdebug.txt","w");
 #define ERROR(x...) fprintf(dfp,x)
#else
 void ERROR(const char *format, ...){}
#endif



#define INFO ERROR


unsigned int eventsNew = 0, sensorsActivated = 0;

//#include "svmp.h"

//SVMPThread *t = NULL;

//void sendSensorActivations() {
//	/*
//	 *if(!t)
//	 *        return;
//	 */
//
//	char activate[6] = { 14, 91, 2, 100, 96, 0 };
//
//	int i;
//	for(i=HANDLES_MIN;i<=HANDLES_MAX;i++) {
//		activate[5] = i;
//		if(sensorsActivated & (1 << i)) {
//			activate[3] = 101;
//		} else {
//			activate[3] = 102;
//		}
//
//		//send(t->fd, activate, 6, 0);
//	}
//}

// timestamp for sensor event
int64_t getTimestamp() {
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return int64_t(t.tv_sec)*1000000000LL + t.tv_nsec;
}

void handlepacket(struct svmp_sensor_event_t *p) {
	//INFO("Got a packet of type %d\n",p->type);
        switch(p->type) {
		case SENSOR_TYPE_ACCELEROMETER:
			events[SENSOR_HANDLE_ACCELEROMETER].timestamp = getTimestamp();
			events[SENSOR_HANDLE_ACCELEROMETER].acceleration.x =  p->value[0];
			events[SENSOR_HANDLE_ACCELEROMETER].acceleration.y = p->value[1]; 
			events[SENSOR_HANDLE_ACCELEROMETER].acceleration.z = p->value[2];
			events[SENSOR_HANDLE_ACCELEROMETER].acceleration.status = p->accuracy;
                        //INFO("Got accel values %f, %f, %f (%d, %d, %d)\n", events[SENSORS_ACCELERATION_HANDLE].data[0], events[SENSORS_ACCELERATION_HANDLE].data[1], events[SENSORS_ACCELERATION_HANDLE].data[2], p->value[0], p->value[1], p->value[2]);
			eventsNew |= 1 << SENSOR_HANDLE_ACCELEROMETER;
			break;
		case SENSOR_TYPE_MAGNETIC_FIELD:
			events[SENSOR_HANDLE_MAGNETIC_FIELD].timestamp = getTimestamp();
			events[SENSOR_HANDLE_MAGNETIC_FIELD].magnetic.x = p->value[0];
			events[SENSOR_HANDLE_MAGNETIC_FIELD].magnetic.y = p->value[1];
			events[SENSOR_HANDLE_MAGNETIC_FIELD].magnetic.z = p->value[2];
			events[SENSOR_HANDLE_MAGNETIC_FIELD].magnetic.status = p->accuracy;
			eventsNew |= 1 << SENSOR_HANDLE_MAGNETIC_FIELD;
			break;
		case SENSOR_TYPE_ORIENTATION:
			events[SENSOR_HANDLE_ORIENTATION].timestamp = getTimestamp();
			events[SENSOR_HANDLE_ORIENTATION].orientation.azimuth = p->value[0];
			events[SENSOR_HANDLE_ORIENTATION].orientation.pitch = p->value[1];
			events[SENSOR_HANDLE_ORIENTATION].orientation.roll = p->value[2];
			events[SENSOR_HANDLE_ORIENTATION].orientation.status = p->accuracy;
			eventsNew |= 1 << SENSOR_HANDLE_ORIENTATION;
			break;
		case SENSOR_TYPE_GYROSCOPE:
			events[SENSOR_HANDLE_GYROSCOPE].timestamp = getTimestamp();
			events[SENSOR_HANDLE_GYROSCOPE].gyro.x = p->value[0];
			events[SENSOR_HANDLE_GYROSCOPE].gyro.y = p->value[1];
			events[SENSOR_HANDLE_GYROSCOPE].gyro.z = p->value[2];
			events[SENSOR_HANDLE_GYROSCOPE].gyro.status = p->accuracy;
			eventsNew |= 1 << SENSOR_HANDLE_GYROSCOPE;
			break;
		case SENSOR_TYPE_LIGHT:
			events[SENSOR_HANDLE_LIGHT].timestamp = getTimestamp();
			events[SENSOR_HANDLE_LIGHT].light = p->value[0];
			eventsNew |= 1 << SENSOR_HANDLE_LIGHT;
			break;
		case SENSOR_TYPE_PRESSURE:
			events[SENSOR_HANDLE_PRESSURE].timestamp = getTimestamp();
			events[SENSOR_HANDLE_PRESSURE].pressure = p->value[0];
			eventsNew |= 1 << SENSOR_HANDLE_PRESSURE;
			break;
//		case SENSOR_TYPE_TEMPERATURE:
//			events[SENSOR_HANDLE_TEMPERATURE].timestamp = getTimestamp();
//			events[SENSOR_HANDLE_TEMPERATURE].temperature = p->value[0];
//			eventsNew |= 1 << SENSOR_HANDLE_TEMPERATURE;
//			break;
		case SENSOR_TYPE_PROXIMITY:
			events[SENSOR_HANDLE_PROXIMITY].timestamp = getTimestamp();
			events[SENSOR_HANDLE_PROXIMITY].distance = p->value[0];
			eventsNew |= 1 << SENSOR_HANDLE_PROXIMITY;
			break;
		case SENSOR_TYPE_GRAVITY:
			events[SENSOR_HANDLE_GRAVITY].timestamp = getTimestamp();
			events[SENSOR_HANDLE_GRAVITY].acceleration.x =  p->value[0];
			events[SENSOR_HANDLE_GRAVITY].acceleration.y = p->value[1]; 
			events[SENSOR_HANDLE_GRAVITY].acceleration.z = p->value[2];
			events[SENSOR_HANDLE_GRAVITY].acceleration.status = p->accuracy;
			eventsNew |= 1 << SENSOR_HANDLE_GRAVITY;
			break;
		case SENSOR_TYPE_LINEAR_ACCELERATION:
			events[SENSOR_HANDLE_LINEAR_ACCELERATION].timestamp = getTimestamp();
			events[SENSOR_HANDLE_LINEAR_ACCELERATION].acceleration.x =  p->value[0];
			events[SENSOR_HANDLE_LINEAR_ACCELERATION].acceleration.y = p->value[1]; 
			events[SENSOR_HANDLE_LINEAR_ACCELERATION].acceleration.z = p->value[2];
			events[SENSOR_HANDLE_LINEAR_ACCELERATION].acceleration.status = p->accuracy;
			eventsNew |= 1 << SENSOR_HANDLE_LINEAR_ACCELERATION;
			break;
		case SENSOR_TYPE_ROTATION_VECTOR:
			events[SENSOR_HANDLE_ROTATION_VECTOR].timestamp = getTimestamp();
			events[SENSOR_HANDLE_ROTATION_VECTOR].acceleration.x =  p->value[0];
			events[SENSOR_HANDLE_ROTATION_VECTOR].acceleration.y = p->value[1]; 
			events[SENSOR_HANDLE_ROTATION_VECTOR].acceleration.z = p->value[2];
			events[SENSOR_HANDLE_ROTATION_VECTOR].acceleration.status = p->accuracy;
			eventsNew |= 1 << SENSOR_HANDLE_ROTATION_VECTOR;
			break;
//		case SENSOR_TYPE_RELATIVE_HUMIDITY:
//			events[SENSOR_HANDLE_RELATIVE_HUMIDITY].timestamp = getTimestamp();
//			events[SENSOR_HANDLE_RELATIVE_HUMIDITY].relative_humidity = p->value[0];
//			eventsNew |= 1 << SENSOR_HANDLE_RELATIVE_HUMIDITY;
//			break;
//		case SENSOR_TYPE_AMBIENT_TEMPERATURE:
//			events[SENSOR_HANDLE_AMBIENT_TEMPERATURE].timestamp = getTimestamp();
//			events[SENSOR_HANDLE_AMBIENT_TEMPERATURE].temperature = p->value[0];
//			eventsNew |= 1 << SENSOR_HANDLE_AMBIENT_TEMPERATURE;
//			break;
		default:
			break;
			//INFO("unknown sensor type: %d\n",p->type);
	}

}



int processPacket(int fd) {

	//struct svmp_sensor_event_t *p = (struct svmp_sensor_event_t*)malloc(sizeof(struct svmp_sensor_event_t));;
	int len = sizeof(struct svmp_sensor_event_t);
	//char buf [64];
	//struct svmp_sensor_event_t *p = NULL; //malloc(sizeof(struct svmp_sensor_event_t));;
	struct svmp_sensor_event_t event;
	int tot = 0;
	int tmp;
	memset(&event,0,sizeof(event));
	ERROR("processing packet \n");
	tot = read(fd,&event,sizeof(event));
	ERROR("processing packet : read %d bytes \n",tot);
	if (tot < 0) {
		ERROR("read error : %s\n",strerror(errno));
		return tot;
	}
	if (tot == 0){
		ERROR("read 0 bytes : %s\n",strerror(errno));
		return tot;
	}

	ERROR("packet received type :%d, len %d\n",event.type,tot);
	handlepacket(&event);

	//free(p);
	return tot;
	// sanity check on the packet

}

void *daemonListenThread(void* ignore) {

	int servfd,newfd,fdmax,
	    i,n;
        socklen_t servlen,clilen;
	fd_set master,read_fds;
	struct sockaddr_un  cli_addr, serv_addr;
	fdmax=0;
	char buf[80];
	ERROR("daemonListenThread\n");	

	servfd = android_get_control_socket("svmp_sensors");
	if (servfd < 0 )
		ERROR("error with android_get_control_socket: error %s\n",strerror(errno));
	else
		ERROR("android_get_control_socket success fd %d\n", servfd);
	

	if(listen(servfd, 10) == -1) {
		//perror("Could not listen to raw daemon socket");
		ERROR("listen error %s\n",strerror(errno));
		pthread_exit(0);
	}

	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	FD_SET(servfd,&master);

	fdmax = max(fdmax,servfd);

	ERROR("starting select loop\n");
	for (;;) {
		read_fds=master;

		if(select(fdmax+1,&read_fds,NULL,NULL,NULL)==-1)
			ERROR("select error %s\n",strerror(errno));

		for(i=0; i<=fdmax; i++){
			if(FD_ISSET(i,&read_fds)) {
				printf("fd is set i=%d, servfd %d \n",i,servfd);
				if(i == servfd) {
					newfd = accept(servfd,NULL,NULL);
					      //servfd,(struct sockaddr *)&cli_addr,&clilen);
					if (newfd >0){
						ERROR("new connection accepted fd = %d\n",newfd);
						//fflush(dfp);
						FD_SET(newfd,&master);
						fdmax=max(fdmax,newfd);
						// initialize sensors
						//sendSensorActivations();
					}
					else 
						ERROR("accept error: %s\n", strerror(errno));
				}else { // existing client
					int n=processPacket(i);
					if (n<=0) {
						ERROR("closing connection to client \n");
						//fflush(dfp);
						close(i);
						FD_CLR(i, &master);
					} else {
						//fflush(dfp);
						pthread_cond_signal(&newpacket_cv);
						ERROR("new packet processed size : %d\n",n);
						//pthread_mutex_unlock(&newpacket_mutex);
					}
				}
			}
		}
	}
	pthread_exit(NULL);
	return 0;

}

int initSockets() {
	int err;
	ERROR("initSockets\n");	
	err=pthread_create(&daemon_thread, NULL, daemonListenThread, NULL);
	if (err < 0) {
		ERROR("pthread_create error %s\n",strerror(errno));	
	}
	return 0;

}

}  // end extern C

/*****************************************************************************/

static int sensors_close(struct hw_device_t *dev) {
    /* Close the sensors. */

	ERROR("Close\n");

	delete dev;
	sensorsActivated = 0;
	//sendSensorActivations();

    return 0;
}

static int sensor_activate(struct sensors_poll_device_t *dev, int handle, int enabled) {
	/* Activate a sensor. */
	ERROR("Activate %d/%d\n", handle,enabled);

	if(handle>=HANDLES_MIN&&handle<=HANDLES_MAX) {
		if(enabled)
			sensorsActivated |= ( 1 << handle );
		else
			sensorsActivated &= ~( 1 << handle );

		//sendSensorActivations();
	}

	return 0;
}

static int sensor_setDelay(struct sensors_poll_device_t *dev, int handle, int64_t ns) {
	/* Set the polling interval on the sensor. */    

	return 0;
}

static int sensors_poll(struct sensors_poll_device_t *dev, sensors_event_t* data, int count) {
    /* Put events into the sensors_event_ts in data, up to count */

	int i, pos=0;
	int err;
	char buf[1];
	struct timeval poll;
	ERROR("Sensors poll \n");

	//sleep(1);
	
	/* We block until a packet is received. */
	pthread_mutex_lock(&newpacket_mutex);
	pthread_cond_wait(&newpacket_cv,&newpacket_mutex);
	pthread_mutex_unlock(&newpacket_mutex);
	
	gettimeofday(&poll, NULL);
	ERROR("Sensors poll: packet arrived! \n");

	// remember when this device last provided an update; force it to 
	// update if it's been more than the device's minimum interval
	for(i=HANDLES_MIN;i<=HANDLES_MAX;i++) {
		if(lastPoll[0].tv_sec == 0)
			continue;
		long long diff = 1000000ll * ( poll.tv_sec - lastPoll[i].tv_sec ) + ( poll.tv_usec - lastPoll[i].tv_usec );
		if(diff >= sSensorList[i].minDelay) {
			eventsNew |= ( 1 << i );		
		}
	}

	for(i=HANDLES_MIN;i<=HANDLES_MAX;i++) {
		if(eventsNew & ( 1 << i )) {
			//INFO("Giving event %d",i);
			data[pos++] = events[i];
			lastPoll[i] = poll;
			ERROR("Sending event values %f, %f, %f\n", events[i].data[0], events[i].data[1], events[i].data[2]);
		}
		if(pos>count)
			break;
	}

	if(eventsNew || pos){
		ERROR("Polling, eventsNew = %d, sensorsActivated = %d, pos = %d\n", eventsNew, sensorsActivated, pos);
	}

	eventsNew = 0;


    return pos;	// # of events filled
}

/*****************************************************************************/

/** Open a new instance of a sensor device using name */
static int open_sensors(const struct hw_module_t* module, const char* id, struct hw_device_t** device) {
	/* initialize sockets, debug */

	ERROR("open_sensors starting");
	ALOGE("open_sensors starting");

	pthread_mutex_init(&newpacket_mutex, NULL);
	pthread_cond_init (&newpacket_cv, NULL);
       
	initSockets();

	/*
	 *if (pipe(pipefd) == -1) {
	 *        ERROR("error initializing pipe\n");
	 *        return(EXIT_FAILURE);
	 *}
	 */
        sensors_poll_device_t *dev = new sensors_poll_device_t();
        memset(dev, 0, sizeof(sensors_poll_device_t));

        dev->common.tag = HARDWARE_DEVICE_TAG;
        dev->common.version  = 0;
        dev->common.module   = const_cast<hw_module_t*>(module);
        dev->common.close    = sensors_close;
        dev->activate        = sensor_activate;
        dev->setDelay        = sensor_setDelay;
        dev->poll            = sensors_poll;

        *device = &dev->common;


        return 0;	// successful
}

static int get_sensors_list(struct sensors_module_t* module,
                                     struct sensor_t const** list) 
{
        *list = sSensorList;
        return sizeof(sSensorList) / sizeof(struct sensor_t);

}

static struct hw_module_methods_t sensors_module_methods = {
        open: open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
        common: {
                tag: HARDWARE_MODULE_TAG,
                version_major: 1,
                version_minor: 0,
                id: SENSORS_HARDWARE_MODULE_ID,
                name: "MOCSI SVMP Fake Sensor Module",
                author: "MITRE",
                methods: &sensors_module_methods,
        },
        get_sensors_list: get_sensors_list,
};
