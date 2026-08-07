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
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

# define MTU 1024 // Maximum transmission unit

struct message_node {
    char header[128];
    char body[MTU];
    struct message_node *next;  // Pointer to the next chunked message
};

struct header_metadata {
    FILE *handle;
    char *uri; 
    int response_no;
    int length;
    int fd;
    int chunk;                  // Require chunk encoding bool
};

struct header {
    const char *response_type;  // Response number as a string
    const char *content_type;   // Content type as a string
    struct header_metadata data;
};

typedef struct message_node Message_Node;
typedef struct header_metadata Header_Metadata;
typedef struct header Header;

/**
 * @brief Build a linked list of response strings
 *        to write back to the socket
 * 
 * @return A list of buffered responses
 */
struct message_node *response_list(char *buffer);

#endif