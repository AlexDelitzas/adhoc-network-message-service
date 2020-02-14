#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#define BODY_SIZE 256
#define MESSAGE_SIZE 300

struct message
{
  uint32_t sender;
  uint32_t receiver;
  uint64_t timestamp;
  char body[BODY_SIZE];
};

typedef struct message message;

void print_message(const message *mes);
message create_message(uint32_t sender, uint32_t receiver, char *body);
short compare_messages(const message *mes1, const message *mes2);
void message_to_char_array(char *str, const message *mes);
void char_array_to_message(message *mes, char *str);

#endif /* MESSAGE_H */
