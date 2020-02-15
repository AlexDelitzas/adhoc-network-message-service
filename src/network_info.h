#ifndef NETWORK_INFO_H
#define NETWORK_INFO_H

#include <stdint.h>

#define DEVICES_FILENAME "../devices_info/device_ids.txt"

// if the device ID is not defined, set it to a default value
#ifndef MY_ID
#define MY_ID 8448
#endif

#define IP_LEN 11

struct devices_in_network_info
{
  uint32_t my_id;
  char my_ip[IP_LEN];
  unsigned int number_of_devices;
  uint32_t *device_ids;
  char (*device_IP)[IP_LEN];
};
typedef struct devices_in_network_info  devices_in_network_info;

devices_in_network_info devices_info;

unsigned int find_number_of_devices();

void get_device_ids(uint32_t **device_ids, unsigned int number_of_devices);

void get_ip_from_id(uint32_t id, char *ip);

int get_device_index_from_ip(const char *ip);

void set_devices_info(devices_in_network_info *devices_info);

void free_devices_in_network_info(devices_in_network_info *devices_info);

#endif /* NETWORK_INFO_H */
