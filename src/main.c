#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include "network_info.h"
#include "message_gen.h"
#include "queue.h"
#include "socket_comm.h"

struct timeval exp_startwtime;

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage: %s t\n", argv[0]);
    printf("\t- t: total duration (seconds)\n");
    exit(EXIT_FAILURE);
  }

  int total_duration = atoi(argv[1]);

  if (total_duration <= 0)
  {
    printf("Error: total duration must be positive\n");
    exit(EXIT_FAILURE);
  }

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGALRM);
  pthread_sigmask(SIG_BLOCK, &set, NULL);

  // get information for all devices in the network
  extern devices_in_network_info devices_info;
  set_devices_info(&devices_info);

  queue *msg_queue = (queue *)malloc(sizeof(queue));
  initialize(msg_queue);

  init_connection_settings();

  // get the date and time that the session started
  time_t rawtime;
  time(&rawtime);
  struct tm *time_info = localtime(&rawtime);

  printf("===================\n");
	printf("[*] Session started: %s\n", asctime(time_info));
  printf("Number of devices in ad-hoc network: %d\n", devices_info.number_of_devices + 1);
  printf("Experiment duration: %d sec\n", total_duration);
  printf("===================\n\n");
  printf("----- Activity Logs -----\n");

  gettimeofday(&exp_startwtime, NULL);

  pthread_t message_generator_thread;
  pthread_t client_mode_thread;
  pthread_t server_mode_thread;

  // create a thread to handle message generation
  pthread_create(&message_generator_thread, NULL, message_generator, (void *) msg_queue);

  // create a thread to handle client mode (sending connection requests to other devices)
  pthread_create(&client_mode_thread, NULL, client_mode_worker, (void *) msg_queue);

  // create a thread to handle server mode (receiving connection requests from other devices)
  pthread_create(&server_mode_thread, NULL, server_mode_worker, (void *) msg_queue);

  struct timeval exp_endwtime;

  // wait for the experiment to complete
  while (1)
  {
    gettimeofday(&exp_endwtime, NULL);
    double exp_duration = (double) (exp_endwtime.tv_sec - exp_startwtime.tv_sec);

    if (exp_duration > total_duration) break;
  }

  printf("\nCancelling threads...\n");

  // stop generating messages
  pthread_cancel(message_generator_thread);
  pthread_join(message_generator_thread, NULL);
  printf("Message generator thread - Cancelled\n");

  // wait for unfinished session to complete 
  unsigned int cancel_wait_time = 5 * (devices_info.number_of_devices + 1);
  printf("Wait for %d sec for all message exchanges to complete\n", cancel_wait_time);
  sleep(cancel_wait_time);
  pthread_cancel(server_mode_thread);
  pthread_cancel(client_mode_thread);

  pthread_join(server_mode_thread, NULL);
  printf("Server thread - Cancelled\n");

  pthread_join(client_mode_thread, NULL);
  printf("Client thread - Cancelled\n");

  printf("\nFreeing resources...\n");

  close_message_gen_logging();

  save_queue_to_file(msg_queue->front);
  display(msg_queue->front);

  free_connection_settings();
  free_queue(msg_queue);
  free_devices_in_network_info(&devices_info);

  // get the date and time that the session ended
  time(&rawtime);
  time_info = localtime(&rawtime);

  printf("===================\n");
  printf("[*] Session ended at: %s\n", asctime(time_info));
  printf("===================\n\n");

  return 0;
}
