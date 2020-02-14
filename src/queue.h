#ifndef QUEUE_H
#define QUEUE_H

#include "message.h"
#include "network_info.h"

#define QUEUE_SIZE 2000

struct queue_item
{
  message mes;

  short int *devices_sent;

  struct queue_item *next;
};

typedef struct queue_item queue_item;


struct queue
{
    int count;
    queue_item *front;
    queue_item *rear;
    short int *has_new_messages;
};
typedef struct queue queue;

extern devices_in_network_info devices_info;

void initialize(queue *q);

short isempty(queue *q);

message dequeue(queue *q);
void enqueue(queue *q, const message *mes, int device_connected_index);

short exists_in_queue(queue *q, const message *mes);

void display(queue_item *head);

void save_queue_to_file(queue_item *head);

void free_queue(queue *q);


#endif /* QUEUE_H */
