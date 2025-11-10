#ifndef USR_SOCKET_H_   
#define USR_SOCKET_H_

# define BACKLOG_LIM 128

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

/**
 * @brief Print the fields of the nth client session socket structure
 * @param res A pointer to the socket structure
 * @param session The number that references the session we are printing
 */
void print_client(struct sockaddr_storage *res, int fd);

/**
 * @brief Initializes an IPv6 client socket by session number and returns its file descriptor
 * @param log_client Flag to request the printing of the client's socket structure
 * @param sockfd The host socket's file descriptor
 * @return The file descriptor of the client on successfull acceptance, else -1
 */
int client_socket_init(int log_client, int sockfd);

/**
 * @brief Prints the fields of a host socket structure
 * @param res A pointer to the socket structure
 */
void print_host(struct addrinfo *res);

/**
 * @brief Initializes an IPv6 host socket and returns its file descriptor
 * @param log_host Flag to request the printing of the host's socket structure
 * @param host_name The host name under which we use an IP for socket communication
 * @param service_name The service we request to run with the socket
 * @return The file descriptor of the host on successfull initialization, else -1
 */
int host_socket_init(int log_host, char *host_name, char *service_name);

#endif // USR_SOCKET