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
#include "svmp_2.h"
#define LOG_TAG "SVMP"
#include <cutils/log.h>

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

/*****************************************************************************/

#define SENSORS_ACCELERATION_HANDLE     0
#define SENSORS_MAGNETIC_FIELD_HANDLE   1

#define HANDLES_MIN						0
#define HANDLES_MAX						1

#define SENSORS_ACCELERATION_MAX	( 4.0f * GRAVITY_EARTH )
#define SENSORS_MAGNETIC_FIELD_MAX	2000.0f
#define CONVERT_A                   (GRAVITY_EARTH / LSB / NUMOFACCDATA)

/*****************************************************************************/


  /////////////////////////
 // All Per-Sensor Data //
/////////////////////////

struct sensors_event_t events[] = {
	{	sizeof(struct sensors_event_t), SENSORS_ACCELERATION_HANDLE, SENSOR_TYPE_ACCELEROMETER, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	},
	{	sizeof(struct sensors_event_t), SENSORS_MAGNETIC_FIELD_HANDLE, SENSOR_TYPE_MAGNETIC_FIELD, 
			0, 0, {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}, {0, 0, 0, 0}	}
};

// last update time of each device
struct timeval lastPoll[] = { { 0, 0 }, { 0, 0 } };

pthread_t daemon_thread;
static const struct sensor_t sSensorList[] = {

	{ "MOCSi SVMP Fake Accelerometer",        // name
	  "MITRE",                                // vendor
	  1,                                      // version
	  SENSORS_ACCELERATION_HANDLE,            // handle
	  SENSOR_TYPE_ACCELEROMETER,              // type
	  SENSORS_ACCELERATION_MAX,               // maxRange
	  SENSORS_ACCELERATION_MAX / 32767.0f,    // resolution
	  0.25f,                                  // power
	  0,                                      // minDelay 
	  {}},                                    // reserved

	 { "MOCSi SVMP Magnetic Field Sensor",    // name
	  "MITRE",                                // vendor
	  1,                                      // version
	  SENSORS_MAGNETIC_FIELD_HANDLE,          // handle
	  SENSOR_TYPE_MAGNETIC_FIELD,             // type
	  SENSORS_MAGNETIC_FIELD_MAX,             // maxRange
	  SENSORS_MAGNETIC_FIELD_MAX / 32767.0f,  // resolution
	  7.0f,                                   // power
	  0,                                      // minDelay 
	  {}}                                     // reserved
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

void sendSensorActivations() {
	/*
	 *if(!t)
	 *        return;
	 */

	char activate[6] = { 14, 91, 2, 100, 96, 0 };

	int i;
	for(i=HANDLES_MIN;i<=HANDLES_MAX;i++) {
		activate[5] = i;
		if(sensorsActivated & (1 << i)) {
			activate[3] = 101;
		} else {
			activate[3] = 102;
		}

		//send(t->fd, activate, 6, 0);
	}
}

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
		case TYPE_ACCELEROMETER:
			events[SENSORS_ACCELERATION_HANDLE].timestamp = getTimestamp();
			events[SENSORS_ACCELERATION_HANDLE].data[0] =  p->value[0];
			events[SENSORS_ACCELERATION_HANDLE].data[1] = p->value[1]; 
			events[SENSORS_ACCELERATION_HANDLE].data[2] = p->value[2];
                        //INFO("Got accel values %f, %f, %f (%d, %d, %d)\n", events[SENSORS_ACCELERATION_HANDLE].data[0], events[SENSORS_ACCELERATION_HANDLE].data[1], events[SENSORS_ACCELERATION_HANDLE].data[2], p->value[0], p->value[1], p->value[2]);
			eventsNew |= 1 << SENSORS_ACCELERATION_HANDLE;
			break;
		case TYPE_MAGNETIC_FIELD:
			events[SENSORS_MAGNETIC_FIELD_HANDLE].timestamp = getTimestamp();
			events[SENSORS_MAGNETIC_FIELD_HANDLE].data[0] = p->value[0];
			events[SENSORS_MAGNETIC_FIELD_HANDLE].data[1] = p->value[1];
			events[SENSORS_MAGNETIC_FIELD_HANDLE].data[2] = p->value[2];
			eventsNew |= 1 << SENSORS_MAGNETIC_FIELD_HANDLE;
			break;

		default:
			break;
			//INFO("unknown sensor type: %d\n",p->type);
	}

}



int processPacket(int fd) {

	//struct svmp_sensor_event_t *p = (struct svmp_sensor_event_t*)malloc(sizeof(struct svmp_sensor_event_t));;
	int len = sizeof(struct svmp_sensor_event_t);
	char buf [64];
	struct svmp_sensor_event_t *p = NULL; //malloc(sizeof(struct svmp_sensor_event_t));;
	int tot = 0;
	int tmp;
	memset(buf,0,sizeof(buf));
	ERROR("processing packet \n");
	tot = read(fd,buf,len);
	ERROR("processing packet : read %d bytes \n",tot);
	if (tot < 0) {
		ERROR("read error : %s\n",strerror(errno));
		return tot;
	}
	if (tot == 0){
		ERROR("read 0 bytes : %s\n",strerror(errno));
		return tot;
	}

	p=(struct svmp_sensor_event_t*)buf;
	ERROR("packet received type :%d, len %d\n",p->type,tot);
	handlepacket(p);

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
						sendSensorActivations();
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
	sendSensorActivations();

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
		ERROR("Polling, eventsNew = %d, sensorsActivated = %d, pos = %d\n", eventsNew, pos, sensorsActivated);
	}

	eventsNew = 0;


    return pos;	// # of events filled
}

/*****************************************************************************/

/** Open a new instance of a sensor device using name */
static int open_sensors(const struct hw_module_t* module, const char* id, struct hw_device_t** device) {
	/* initialize sockets, debug */

	ERROR("open_sensors starting");
	LOGE("open_sensors starting");

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
