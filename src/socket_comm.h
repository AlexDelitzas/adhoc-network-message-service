#ifndef SOCKET_COMM_H
#define SOCKET_COMM_H

#include "message.h"
#include "queue.h"
#include <pthread.h>

#define PORT 2288
#define BUFFER_SIZE MESSAGE_SIZE
#define SOCKET_LISTEN_QUEUE_LEN 5
#define HANDSHAKE_BUFFER_SIZE 10


short int *active_connections;
FILE *fp_logs;
extern devices_in_network_info devices_info;
extern struct timeval exp_startwtime;

pthread_mutex_t msg_queue_lock;
pthread_mutex_t write_logs_lock;
pthread_mutex_t active_connections_lock;

static long unsigned int session_num;

struct args_new_connection {
    int sockfd;
    int device_connected_index;
    queue *msg_queue;
    short int mode;
};
typedef struct args_new_connection args_new_connection;

void init_connection_settings();

void *server_mode_worker(void *args);
void *client_mode_worker(void *args);

void server_mode(queue *q);
short int client_mode(queue *q, int device_to_connect_index);

void *exchange_messages_worker(void *args);
short int handshake(int sockfd, int mode);

short int send_message(int sockfd, const message *mes);
short int receive_message(int newsockfd, message *mes);

void send_new_messages(int sockfd, queue *q, int device_connected_index, char *buffer_for_logs);
void receive_new_messages(int newsockfd, queue *q, int device_connected_index, char *buffer_for_logs);

uint32_t random_interval(uint32_t min, uint32_t max);

void free_connection_settings();

#endif /* SOCKET_COMM_H */
