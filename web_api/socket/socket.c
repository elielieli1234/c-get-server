#include "socket.h"

# define BACKLOG_LIM 128

void print_client(struct sockaddr_storage *res, int fd) {
    struct sockaddr_in6 *client_addr = (struct sockaddr_in6 *) res;
    char ip6[INET6_ADDRSTRLEN];
    int port_no;
    
    // Print client details
    printf("Client connection %d\n", fd); 
    printf("    Port number      : %u\n", htons(client_addr->sin6_port));
    
    // Print IPv6 Address
    memset(ip6, 0, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &(client_addr->sin6_addr), ip6, INET6_ADDRSTRLEN);
    printf("    IPv6 Address     : %s\n", ip6); 
}

int client_socket_init(int log_client, int sockfd) {
    struct sockaddr_storage client_addr;
    int addr_size = sizeof(struct sockaddr_storage);

    int client_fd = accept(sockfd, (struct sockaddr *) &client_addr, (socklen_t *) &addr_size);
    if (client_fd == -1) {
        fprintf(stderr, "[ERROR]: accept\n");
        return -1;
    }

    if (log_client) print_client(&client_addr, client_fd);

    return client_fd;
}

void print_host(struct addrinfo *res) {
    printf("Host connection\n");
    printf("    Address family   : %d\n", res->ai_family);
    printf("    Socket type      : %d\n", res->ai_socktype);
    printf("    Socket protocol  : %d\n", res->ai_protocol);   

    struct sockaddr_in6 *addr = (struct sockaddr_in6 *) res->ai_addr;
    unsigned short port_no = htons(addr->sin6_port);
    printf("    Port number      : %u\n", port_no);
    
    char ip6[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(addr->sin6_addr), ip6, INET6_ADDRSTRLEN);
    printf("    IPv6 Address     : %s\n\n", ip6);
}

int host_socket_init(int log_host, char *host_name, char *service_name) {
    struct addrinfo hints, *res, *res0;
    int sockfd;
    int status;

    // Get host socket info at head of linked list
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;     // IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // Fill in my IP for me

    if ((status = getaddrinfo(host_name, service_name, &hints, &res)) != 0) {
        fprintf(stderr, "[ERROR]: getaddrinfo");
        return -1;
    }

    if (log_host) print_host(res);

    // Create socket from first valid struct in list
    for (res0 = res; res0 != NULL; res0 = res0->ai_next) {
        sockfd = socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol);
        if (sockfd == -1) continue;
        else break;
    }
    if (sockfd == -1) {
        fprintf(stderr, "[ERROR]: socket\n");
        freeaddrinfo(res);
        return -1;
    }

    // Bind and listen
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)); // Make socket available for reuse
    if (bind(sockfd, res0->ai_addr, res0->ai_addrlen) == -1) {
        fprintf(stderr, "[ERROR]: bind\n");
        freeaddrinfo(res);
        return -1;                
    }
    if (listen(sockfd, BACKLOG_LIM) == -1) { // Create backlog of uninitialized connections
        fprintf(stderr, "[ERROR]: listen\n");
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res); // We no longer need host socket info
    return sockfd;
}