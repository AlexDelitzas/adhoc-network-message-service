#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>


#include "socket_comm.h"

void error(const char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

uint32_t random_interval(uint32_t min, uint32_t max)
{
    return (rand() % (max - min + 1)) + min;
}

void init_connection_settings()
{
  active_connections = (short int *)malloc(devices_info.number_of_devices * sizeof(short int));

  if (active_connections == NULL)
  {
    fprintf(stderr, "Error: Memory not allocated\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < devices_info.number_of_devices; i++)
  {
    active_connections[i] = 0;
  }

  if (pthread_mutex_init(&msg_queue_lock, NULL) != 0)
  {
      printf("Mutex init has failed\n");
      exit(EXIT_FAILURE);
  }
  if (pthread_mutex_init(&write_logs_lock, NULL) != 0)
  {
      printf("Mutex init has failed\n");
      exit(EXIT_FAILURE);
  }
  if (pthread_mutex_init(&active_connections_lock, NULL) != 0)
  {
      printf("Mutex init has failed\n");
      exit(EXIT_FAILURE);
  }

  // Use current time as seed for random generator
  srand(time(0));

  char filename[50];
  sprintf(filename, "../logs/sessions_%d.json", MY_ID);
  fp_logs = fopen(filename, "w");

  if (!fp_logs)
	{
		perror("Could not write to file");
		exit(EXIT_FAILURE);
	}

  fprintf(fp_logs, "{\n\t\"sessions\": [");
}

short int send_message(int sockfd, const message *mes)
{
  char buffer[BUFFER_SIZE];
  int n;

  bzero(buffer, BUFFER_SIZE);
  message_to_char_array(buffer, mes);

  n = send(sockfd, buffer, MESSAGE_SIZE, 0);

  if (n < 0)
    perror("ERROR writing to socket");

  return (n > 0);
}

short int receive_message(int newsockfd, message *mes)
{
  char buffer[BUFFER_SIZE];
  int n;

  bzero(buffer, BUFFER_SIZE);

  n = recv(newsockfd, buffer, MESSAGE_SIZE, 0);

  if (n < 0)
    perror("ERROR reading from socket");
  else if (n > 0)
    char_array_to_message(mes, buffer);

  return (n > 0);
}

void send_new_messages(int sockfd, queue *q, int device_connected_index, char *buffer_for_logs)
{
  queue_item *item_to_send;
  item_to_send = q->front;

  short int is_first_message = 1;

  while (item_to_send != NULL)
  {
    short int msg_was_sent = 0;
    short int sent_successfully = 0;

    pthread_mutex_lock(&msg_queue_lock);

    message *cur_msg = &(item_to_send->mes);
    if (item_to_send->devices_sent[device_connected_index] == 0)
    {
      msg_was_sent = 1;
      if (send_message(sockfd, &(item_to_send->mes)))
      {
        item_to_send->devices_sent[device_connected_index] = 1;
        sent_successfully = 1;
      }
    }

    item_to_send = item_to_send->next;
    if (item_to_send == NULL)
    {
      q->has_new_messages[device_connected_index] = 0;
    }
    pthread_mutex_unlock(&msg_queue_lock);

    if (msg_was_sent)
    {
      char current_message[MESSAGE_SIZE];
      message_to_char_array(current_message, cur_msg);

      if (is_first_message)
      {
        sprintf(buffer_for_logs,
          "%s\n\t\t\t\t{\n\t\t\t\t\t\"message\": \"%s\",\n\t\t\t\t\t\"success\": %d\n\t\t\t\t}",
          buffer_for_logs, current_message, sent_successfully);

        is_first_message = 0;
      }
      else
      {
        sprintf(buffer_for_logs,
          "%s,\n\t\t\t\t{\n\t\t\t\t\t\"message\": \"%s\",\n\t\t\t\t\t\"success\": %d\n\t\t\t\t}",
          buffer_for_logs, current_message, sent_successfully);
      }
    }
  }
}

void receive_new_messages(int newsockfd, queue *q, int device_connected_index, char *buffer_for_logs)
{
  short int is_first_message = 1;

  while (1)
  {
    message mes;
    if (!receive_message(newsockfd, &mes))
    {
      break;
    }

    short int already_exists = 0;
    pthread_mutex_lock(&msg_queue_lock);
    if (!exists_in_queue(q, &mes))
    {
      enqueue(q, &mes, device_connected_index);
    }
    else
    {
      already_exists = 1;
    }
    pthread_mutex_unlock(&msg_queue_lock);

    char current_message[MESSAGE_SIZE];
    message_to_char_array(current_message, &mes);

    if (is_first_message)
    {
      sprintf(buffer_for_logs,
        "%s\n\t\t\t\t{\n\t\t\t\t\t\"message\": \"%s\",\n\t\t\t\t\t\"already_exists\": %d\n\t\t\t\t}",
        buffer_for_logs, current_message, already_exists);

      is_first_message = 0;
    }
    else
    {
      sprintf(buffer_for_logs,
        "%s,\n\t\t\t\t{\n\t\t\t\t\t\"message\": \"%s\",\n\t\t\t\t\t\"already_exists\": %d\n\t\t\t\t}",
        buffer_for_logs, current_message, already_exists);
    }

  }
}

short int handshake(int sockfd, int mode)
{
  char buffer[HANDSHAKE_BUFFER_SIZE];
  int n;
  char request_message[] = "START";
  char response_message[] = "ACK";

  bzero(buffer, HANDSHAKE_BUFFER_SIZE);

  // server mode
  if (mode == 1)
  {
    n = recv(sockfd, buffer, HANDSHAKE_BUFFER_SIZE, 0);

    if (n < 0)
    {
      perror("ERROR reading from socket");
      return 0;
    }

    if (n == 0 || strcmp(buffer, request_message)) return 0;

    bzero(buffer, HANDSHAKE_BUFFER_SIZE);
    strcpy(buffer, response_message);
    n = send(sockfd, buffer, HANDSHAKE_BUFFER_SIZE, 0);
    if (n < 0)
    {
      perror("ERROR writing to socket");
      return 0;
    }
  }

  // client mode
  else
  {
    strcpy(buffer, request_message);
    n = send(sockfd, buffer, HANDSHAKE_BUFFER_SIZE, 0);
    if (n < 0)
    {
      perror("ERROR writing to socket");
      return 0;
    }

    bzero(buffer, HANDSHAKE_BUFFER_SIZE);
    n = recv(sockfd, buffer, HANDSHAKE_BUFFER_SIZE, 0);

    if (n < 0)
    {
      perror("ERROR reading from socket");
      return 0;
    }

    if (n == 0 || strcmp(buffer, response_message)) return 0;

  }

  return 1;
}

void *exchange_messages_worker(void *args)
{
  args_new_connection *args_ = (args_new_connection *) args;

  int sockfd = args_->sockfd;
  int device_connected_index = args_->device_connected_index;
  queue *q = args_->msg_queue;
  short int mode = args_->mode;

  struct timeval session_startwtime, session_endwtime;

  char buffer_for_logs[1024 * 100];



  pthread_mutex_lock(&active_connections_lock);
  short int connection_already_exists = active_connections[device_connected_index];
  if (!connection_already_exists)
  {
    active_connections[device_connected_index] = 1;
  }
  pthread_mutex_unlock(&active_connections_lock);

  gettimeofday(&session_startwtime, NULL);

  double started_at = (double)((session_startwtime.tv_usec - exp_startwtime.tv_usec)/1.0e6
				      + session_startwtime.tv_sec - exp_startwtime.tv_sec);

  sprintf(buffer_for_logs,
    "\n\t\t{\n\t\t\t\"started_at\": %.6f,\n\t\t\t\"paired_device\": %u",
    started_at, devices_info.device_ids[device_connected_index]);

  short int handshake_done = 0;
  if (!connection_already_exists)
  {
    handshake_done = handshake(sockfd, mode);

    if (!handshake_done)
    {
      printf("[!!] Handshake failed\n");
    }
  }


  short int message_exchange_condition = (!connection_already_exists && handshake_done);
  // server mode
  if (mode == 1)
  {
    // receive messages
    sprintf(buffer_for_logs,
      "%s,\n\t\t\t\"messages_received\": [",
      buffer_for_logs);
    if (message_exchange_condition)
      receive_new_messages(sockfd, q, device_connected_index, buffer_for_logs);
    shutdown(sockfd, SHUT_RD);

    // send messages
    sprintf(buffer_for_logs,
      "%s\n\t\t\t],\n\t\t\t\"messages_sent\": [",
      buffer_for_logs);
    if (message_exchange_condition)
      send_new_messages(sockfd, q, device_connected_index, buffer_for_logs);
    shutdown(sockfd, SHUT_WR);
  }
  // client mode
  else
  {
    // send messages
    sprintf(buffer_for_logs,
      "%s,\n\t\t\t\"messages_sent\": [",
      buffer_for_logs);
    if (message_exchange_condition)
      send_new_messages(sockfd, q, device_connected_index, buffer_for_logs);
    shutdown(sockfd, SHUT_WR);

    // receive messages
    sprintf(buffer_for_logs,
      "%s\n\t\t\t],\n\t\t\t\"messages_received\": [",
      buffer_for_logs);
    if (message_exchange_condition)
      receive_new_messages(sockfd, q, device_connected_index, buffer_for_logs);
    shutdown(sockfd, SHUT_RD);
  }

  close(sockfd);

  pthread_mutex_lock(&active_connections_lock);
  if (!connection_already_exists)
  {
    active_connections[device_connected_index] = 0;
  }
  pthread_mutex_unlock(&active_connections_lock);

  gettimeofday(&session_endwtime, NULL);
  double duration = (double)((session_endwtime.tv_usec - session_startwtime.tv_usec)/1.0e3
				      + (session_endwtime.tv_sec - session_startwtime.tv_sec)/1.0e-3);

  sprintf(buffer_for_logs,
    "%s\n\t\t\t],\n\t\t\t\"duration_msec\": %.6f\n\t\t}",
    buffer_for_logs, duration);



  pthread_mutex_lock(&write_logs_lock);
  long unsigned int current_session_num = ++session_num;

  if (current_session_num == 1)
    fprintf(fp_logs, "%s", buffer_for_logs);
  else
    fprintf(fp_logs, ",%s", buffer_for_logs);

  pthread_mutex_unlock(&write_logs_lock);


}

void server_mode(queue *q)
{
  int sockfd, newsockfd;
  struct sockaddr_in serv_addr, cli_addr;
  int clilen = sizeof(cli_addr);
  int opt = 1;

  // Creating socket file descriptor
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
  {
    error("ERROR opening socket");
  }

  // Forcefully attaching socket to the PORT
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                &opt, sizeof(opt)))
  {
    error("ERROR setsockopt");
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(devices_info.my_ip); // inet_addr("10.0.84.48")
  serv_addr.sin_port = htons(PORT);

  // Forcefully attaching socket to the PORT
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
    error("ERROR on binding");
  }

  if (listen(sockfd, SOCKET_LISTEN_QUEUE_LEN) < 0)
  {
    error("listen");
  }

  while (1)
  {
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error("ERROR on accept");

    //  - get client address
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(cli_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("[*] Established connection with %s (server mode)\n\n", client_ip);

    int device_connected_index = get_device_index_from_ip(client_ip);

    if (device_connected_index == -1)
    {
      printf("Error: Cannot identify the sender's IP\n");
      close(newsockfd);
      continue;
    }

    args_new_connection args = {
      .sockfd = newsockfd,
      .device_connected_index = device_connected_index,
      .msg_queue = q,
      .mode = 1 // server mode
    };

    exchange_messages_worker((void *) &args);
  }

}


short int client_mode(queue *q, int device_to_connect_index)
{
  int sockfd;
  struct sockaddr_in serv_addr;
  struct timeval session_startwtime, session_endwtime;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(devices_info.device_IP[device_to_connect_index]);
  serv_addr.sin_port = htons(PORT);

  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
  {
    close(sockfd);
    return 0;
  }

  args_new_connection args = {
    .sockfd = sockfd,
    .device_connected_index = device_to_connect_index,
    .msg_queue = q,
    .mode = 2 // client mode
  };


  exchange_messages_worker((void *) &args);

  return 1;

}


void *server_mode_worker(void *args)
{
  queue *msg_queue = (queue *) args;

  server_mode(msg_queue);
}

void *client_mode_worker(void *args)
{
  queue *msg_queue = (queue *) args;

  while (1)
  {
    for (int dev_index = 0; dev_index < devices_info.number_of_devices; dev_index++)
    {
      if (active_connections[dev_index] || !msg_queue->has_new_messages[dev_index])
        continue;

      printf("[@] Trying to connect with %s (client mode)\n", devices_info.device_IP[dev_index]);
      if (client_mode(msg_queue, dev_index))
      {
        printf("[*] Successful connection with %s (client mode)\n\n", devices_info.device_IP[dev_index]);
      }
      else
      {
        printf("[!] Could not establish connection with %s (client mode)\n", devices_info.device_IP[dev_index]);
      }
    }
    usleep(random_interval(500000, 999999));

  }

}

void free_connection_settings()
{
  fprintf(fp_logs, "\n\t]\n}");
  fclose(fp_logs);

  free(active_connections);

  pthread_mutex_destroy(&msg_queue_lock);
  pthread_mutex_destroy(&write_logs_lock);
  pthread_mutex_destroy(&active_connections_lock);
}
