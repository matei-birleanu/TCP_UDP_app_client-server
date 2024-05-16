#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>

/*
 * Macro de verificare a erorilor
 * Exemplu:
 * 		int fd = open (file_name , O_RDONLY);
 * 		DIE( fd == -1, "open failed");
 */

#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#endif


typedef struct msg {
    char topic[51];
    char type[15]; 
    char content[1501];
    char ip_udp[17];
    int port;
} msg;

typedef struct client {
  char id[1025];
  int fd;
  int nrt;
  char topics[30][51];
  int connected;
} client;

struct udp_message {
    char topic[51];
    uint8_t data_type;
    char body[1501];
};
