#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <math.h>

# define PACK_LIM 4096 // Max # bytes we send or recieve at once over the wire

struct packet_node {
    size_t length;
    char *packet;
    struct packet_node *next;
};

struct head {
    int response_no;
    const char *response;
    const char *content_type;
    int content_length;
};

/**
 * @brief On invalid HTTP requests assume functionality of an echo server
 * @param buffer The request packet read over the listner's socket
 */
char *echo_fallback(char *buffer);

/**
 * @brief Parse a web request and generate a full packet
 * @param buffer The web request packet
 * @param response_length The length of the response packet
 * @return The full response packet sent back over the socket
 */
char *host_response(char *buffer);

// struct packet_node *construct_packet_list(FILE *requested_asset, struct head fields, int *packet_length);

struct packet_node *response_list(char *buffer);

#endif