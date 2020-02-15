#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network_info.h"

// get the number of other devices in the network
unsigned int find_number_of_devices()
{
  unsigned int number_of_devices = 0;

  FILE *fp;
  char *line_buf = NULL;
  size_t line_buf_size = 0;
  ssize_t line_size;

  fp = fopen(DEVICES_FILENAME, "r");
  if (!fp)
  {
    fprintf(stderr, "Error opening file '%s'\n", DEVICES_FILENAME);
    exit(EXIT_FAILURE);
  }

  // get the first line of the file
  line_size = getline(&line_buf, &line_buf_size, fp);

  /* Loop until the end of the file. */
  while (line_size >= 0)
  {
    number_of_devices++;

    // get the next line
    line_size = getline(&line_buf, &line_buf_size, fp);
  }

  free(line_buf);

  fclose(fp);

  // return the number of the other devices in the network
  return (number_of_devices-1);
}

// get the IDs of the devices in the ad hoc network
void get_device_ids(uint32_t **device_ids, unsigned int number_of_devices)
{
  FILE *fp;
  char *line_buf = NULL;
  size_t line_buf_size = 0;
  int device_index = 0;
  ssize_t line_size;

  *device_ids = (uint32_t *)malloc(number_of_devices * sizeof(uint32_t));

  if (*device_ids == NULL)
  {
    fprintf(stderr, "Error: Memory not allocated\n");
    exit(EXIT_FAILURE);
  }

  fp = fopen(DEVICES_FILENAME, "r");
  if (!fp)
  {
    fprintf(stderr, "Error: Cannot open file '%s'\n", DEVICES_FILENAME);
    exit(EXIT_FAILURE);
  }

  // get the first line of the file
  line_size = getline(&line_buf, &line_buf_size, fp);

  // read the device IDs from the file
  while (line_size >= 0)
  {
    uint32_t id = (uint32_t) atol(line_buf);

    if (id != MY_ID)
    {
      (*device_ids)[device_index] = (uint32_t) atol(line_buf);

      device_index++;
    }

    // get the next line
    line_size = getline(&line_buf, &line_buf_size, fp);
  }

  free(line_buf);

  fclose(fp);
}

// get a device IP from its ID
void get_ip_from_id(uint32_t id, char *ip)
{
  if (id < 1000 || id > 9999)
  {
    fprintf(stderr, "Error: Device ID must be a 4-digit number\n");
    return;
  }

  uint8_t ip_part3 = id / 100;
  uint8_t ip_part4 = id % 100;

  sprintf(ip, "10.0.%hhu.%hhu", ip_part3, ip_part4);
}

// find the position of a device in the devices_info.device_IP array
int get_device_index_from_ip(const char *ip)
{
  for (int i = 0; i < devices_info.number_of_devices; i++)
  {
    if (!strcmp(devices_info.device_IP[i], ip))
    {
      return i;
    }
  }

  return -1;
}

// fill the struct devices_in_network_info
void set_devices_info(devices_in_network_info *devices_info)
{
  devices_info->my_id = MY_ID;

  get_ip_from_id(MY_ID, devices_info->my_ip);

  devices_info->number_of_devices = find_number_of_devices();

  get_device_ids(&(devices_info->device_ids), devices_info->number_of_devices);

  devices_info->device_IP = malloc((devices_info->number_of_devices) * sizeof(*(devices_info->device_IP)));

  if (devices_info->device_IP == NULL)
  {
    fprintf(stderr, "Error: Memory not allocated\n");
    exit(EXIT_FAILURE);
  }

  for (int device_index = 0; device_index < devices_info->number_of_devices; device_index++)
  {
    get_ip_from_id(devices_info->device_ids[device_index], devices_info->device_IP[device_index]);
  }
}

void free_devices_in_network_info(devices_in_network_info *devices_info)
{
  free(devices_info->device_ids);
  free(devices_info->device_IP);
}
