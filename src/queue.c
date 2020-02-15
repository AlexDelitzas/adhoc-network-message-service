#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "queue.h"


void initialize(queue *q)
{
  q->count = 0;
  q->front = NULL;
  q->rear = NULL;

  q->has_new_messages = (short int *)malloc(devices_info.number_of_devices * sizeof(short int));

  if (q->has_new_messages == NULL)
  {
    fprintf(stderr, "Error: Memory not allocated\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < devices_info.number_of_devices; i++)
  {
    q->has_new_messages[i] = 0;
  }
}

short isempty(queue *q)
{
  return (q->count == 0);
}

short exists_in_queue(queue *q, const message *mes)
{
  queue_item *current_item = q->front;

  while (current_item != NULL)
  {
    if (compare_messages(&(current_item->mes), mes))
    {
      return 1;
    }

    current_item = current_item->next;
  }

  return 0;
}

message dequeue(queue *q)
{
  if (isempty(q))
  {
    printf("The queue is empty\n");
    exit(EXIT_FAILURE);
  }

  queue_item *tmp;
  message mes = q->front->mes;
  tmp = q->front;
  q->front = q->front->next;
  q->count--;

  free(tmp->devices_sent);
  free(tmp);

  return mes;
}

void enqueue(queue *q, const message *mes, int device_connected_index)
{
  if (q->count >= QUEUE_SIZE)
  {
    dequeue(q);
  }

  queue_item *new_item;
  new_item = (queue_item *)malloc(sizeof(queue_item));

  if (new_item == NULL)
  {
    fprintf(stderr, "Error: Memory not allocated\n");
    exit(EXIT_FAILURE);
  }

  new_item->mes = *mes;
  new_item->next = NULL;

  new_item->devices_sent = (short int *)malloc(devices_info.number_of_devices * sizeof(short int));

  if (new_item->devices_sent == NULL)
  {
    fprintf(stderr, "Error: Memory not allocated\n");
    exit(EXIT_FAILURE);
  }

  uint32_t final_destination_id = mes->receiver;

  if (final_destination_id != MY_ID)
  {
    for (int dev_idx = 0; dev_idx < devices_info.number_of_devices; dev_idx++)
    {
      new_item->devices_sent[dev_idx] = 0;
    }

    if (device_connected_index != -1)
    {
      new_item->devices_sent[device_connected_index] = 1;
    }
  }
  else
  {
    for (int dev_idx = 0; dev_idx < devices_info.number_of_devices; dev_idx++)
    {
      new_item->devices_sent[dev_idx] = 1;
    }
  }

  if(!isempty(q))
  {
    q->rear->next = new_item;
    q->rear = new_item;
  }
  else
  {
    q->front = q->rear = new_item;
  }

  q->count++;

  // notify that there is a new message for transmission
  if (final_destination_id != MY_ID)
  {
    for (int i = 0; i < devices_info.number_of_devices; i++)
    {
      if (i != device_connected_index)
      {
        q->has_new_messages[i] = 1;
      }
    }
  }

}


void display(queue_item *head)
{
  if(head == NULL)
  {
    printf("--END--\n");
  }
  else
  {
    print_message(&(head->mes));

    display(head->next);
  }
}

void save_queue_to_file(queue_item *head)
{
  FILE *fp;
  char filename[50];
  sprintf(filename, "../logs/queue_%d.txt", MY_ID);
  fp = fopen(filename, "w");

  if (!fp)
	{
		perror("Could not write to file");
		exit(EXIT_FAILURE);
	}

  while (head != NULL)
  {
    char mes_char_array[MESSAGE_SIZE];
    message_to_char_array(mes_char_array, &(head->mes));
    fprintf(fp, "%s\n", mes_char_array);

    head = head->next;
  }

  fclose(fp);

}

void free_queue(queue *q)
{
  while (!isempty(q))
  {
    dequeue(q);
  }

  free(q->has_new_messages);
  free(q);
}
