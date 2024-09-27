// data.h
#ifndef DATA_H
#define DATA_H

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <float.h> // For DBL_MAX (maximum double value)

#include "../project_config.h"

typedef struct SensorData
{
    int raw_data;
    double temperatures[4];  // Array for temperatures
    int heaters[4];          // Array for heaters
    char state_heater[4][8]; // Adjust size as necessary for the timestamp
} sensor;

typedef struct
{
    // Add fields as needed for your specific use case
    sensor *s;
    int desligar_ligar; // Example field
    double set_point;
    double frequencia;

    // Other fields...
} thread_args_t;

#endif // DATA_H
