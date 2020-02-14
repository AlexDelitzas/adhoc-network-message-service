#ifndef MESSAGE_GEN_H
#define MESSAGE_GEN_H


#include <sys/time.h>
#include "queue.h"
#include "network_info.h"

#include <signal.h>
#include <pthread.h>

#define MIN_TO_SEC 60
#define MIN_TIME (1*MIN_TO_SEC)
#define MAX_TIME (5*MIN_TO_SEC)

extern devices_in_network_info devices_info;
extern pthread_mutex_t msg_queue_lock;

struct itimerval timer;
queue *msg_queue;

struct timeval startwtime, endwtime;

FILE *fp_msg_creation_times;

void generate_message(int sig);

void set_timer();

int random_number(int min, int max);

void *message_generator(void *args);

void close_message_gen_logging();

#endif /* MESSAGE_GEN_H */
