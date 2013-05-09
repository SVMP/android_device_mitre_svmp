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


#ifndef SVMP_2_H_HU1RC6LK
#define SVMP_2_H_HU1RC6LK
  /**
     * A constant describing an accelerometer sensor type. See
     * {@link android.hardware.SensorEvent#values SensorEvent.values} for more
     * details.
     */
#define TYPE_ACCELEROMETER 1

/**
 * A constant describing a magnetic field sensor type. See
 * {@link android.hardware.SensorEvent#values SensorEvent.values} for more
 * details.
 */
#define TYPE_MAGNETIC_FIELD 2

/**
 * A constant describing an orientation sensor type. See
 * {@link android.hardware.SensorEvent#values SensorEvent.values} for more
 * details.
 *
 * @deprecated use {@link android.hardware.SensorManager#getOrientation
 *             SensorManager.getOrientation()} instead.
 */
#define TYPE_ORIENTATION 3

/** A constant describing a gyroscope sensor type */
#define TYPE_GYROSCOPE 4

/**
 * A constant describing a light sensor type. See
 * {@link android.hardware.SensorEvent#values SensorEvent.values} for more
 * details.
 */
#define TYPE_LIGHT 5

/** A constant describing a pressure sensor type */
#define TYPE_PRESSURE 6

/**
 * A constant describing a temperature sensor type
 *
 * @deprecated use
 *             {@link android.hardware.Sensor#TYPE_AMBIENT_TEMPERATURE
 *             Sensor.TYPE_AMBIENT_TEMPERATURE} instead.
 */
#define TYPE_TEMPERATURE 7

/**
 * A constant describing a proximity sensor type. See
 * {@link android.hardware.SensorEvent#values SensorEvent.values} for more
 * details.
 */
#define TYPE_PROXIMITY 8

/**
 * A constant describing a gravity sensor type.
 * See {@link android.hardware.SensorEvent SensorEvent}
 * for more details.
 */
#define TYPE_GRAVITY 9

/**
 * A constant describing a linear acceleration sensor type.
 * See {@link android.hardware.SensorEvent SensorEvent}
 * for more details.
 */
#define TYPE_LINEAR_ACCELERATION 10

/**
 * A constant describing a rotation vector sensor type.
 * See {@link android.hardware.SensorEvent SensorEvent}
 * for more details.
 */
#define TYPE_ROTATION_VECTOR 11

/**
 * A constant describing a relative humidity sensor type.
 * See {@link android.hardware.SensorEvent SensorEvent}
 * for more details.
 */
#define TYPE_RELATIVE_HUMIDITY 12

/** A constant describing an ambient temperature sensor type */
#define TYPE_AMBIENT_TEMPERATURE 13

struct svmp_sensor_event_t {
	int type;
	int accuracy;
	long timestamp;
	float value[3];
};

#define SVMPDEBUG 0

//static int pipefd[2];

#endif /* end of include guard: SVMP_2_H_HU1RC6LK */

