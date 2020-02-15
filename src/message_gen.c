#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "message_gen.h"
#include "message.h"
#include "network_info.h"


int random_number(int min, int max)
{
  return (rand() % (max - min + 1)) + min;
}

void generate_message(int sig)
{
  gettimeofday(&endwtime, NULL);

  // create a message to a random recipient
  int random_receiver_index = random_number(0, devices_info.number_of_devices-1);
  message mes = create_message(MY_ID, devices_info.device_ids[random_receiver_index], "Hello world");

  // add message in the queue 
  pthread_mutex_lock(&msg_queue_lock);
  enqueue(msg_queue, &mes, -1);
  pthread_mutex_unlock(&msg_queue_lock);

  double delay = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
  			      + endwtime.tv_sec - startwtime.tv_sec);

  char mes_char_array[MESSAGE_SIZE];
  message_to_char_array(mes_char_array, &mes);
  fprintf(fp_msg_creation_times, "%s, %.8lf\n", mes_char_array, delay);
}

void set_timer()
{
  // initialize timer parameters
  timer.it_value.tv_sec = random_number(MIN_TIME, MAX_TIME);
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;

  // start timer
  if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
    perror("error calling setitimer()");
    exit(EXIT_FAILURE);
  }
  gettimeofday(&startwtime, NULL);
}

void *message_generator(void *args)
{
  msg_queue = (queue *) args;

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGALRM);

  pthread_sigmask(SIG_UNBLOCK, &set, NULL);

  // Use current time as seed for random generator
  srand(time(0));

  // Set handler for SIGALRM signal (required for the timer operation)
  struct sigaction sa;
  memset(&sa, 0, sizeof (sa));
  sa.sa_handler = &generate_message;
  sigaction(SIGALRM, &sa, NULL);

  char filename[50];
  sprintf(filename, "../logs/msg_creation_timestamps_%d.txt", MY_ID);
  fp_msg_creation_times = fopen(filename, "w");

  if (!fp_msg_creation_times)
  {
    perror("Could not write to file");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    set_timer();
    pause();
  }

}

void close_message_gen_logging()
{
  fclose(fp_msg_creation_times);
}
