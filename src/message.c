#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "message.h"

void print_message(const message *mes)
{
  printf("%u_%u_%llu_%s\n", mes->sender, mes->receiver, mes->timestamp, mes->body);
}

message create_message(uint32_t sender, uint32_t receiver, char *body)
{
  message mes;
  mes.sender = sender;
  mes.receiver = receiver;

  strcpy(mes.body, body);

  mes.timestamp = time(NULL);

  return mes;
}

short compare_messages(const message *mes1, const message *mes2)
{
  if (mes1->sender == mes2->sender &&
    mes1->receiver == mes2->receiver &&
    mes1->timestamp == mes2->timestamp &&
    !strcmp(mes1->body, mes2->body))
  {
    return 1;
  }

  return 0;
}

void message_to_char_array(char *str, const message *mes)
{
  sprintf(str, "%u_%u_%llu_%s", mes->sender, mes->receiver, mes->timestamp, mes->body);
}

void char_array_to_message(message *mes, char *str)
{
  const char delim[2] = "_";
  char *token;
  token = strtok(str, delim);

  mes->sender = (uint32_t) strtoul(token, NULL, 10);

  token = strtok(NULL, delim);

  mes->receiver = (uint32_t) strtoul(token, NULL, 10);

  token = strtok(NULL, delim);

  mes->timestamp = (uint64_t) strtoull(token, NULL, 10);

  token = strtok(NULL, delim);

  strcpy(mes->body, token);
}
