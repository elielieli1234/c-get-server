#include "http.h"

char *echo_fallback(char *buffer) {
    return buffer;
}

void url_decode(char *dst, char *src) {
    int path_length = strlen(src), i, j = 0;
    for (i = 0; i < path_length - 2; i++) {
        if (src[i] == '%') {
            char upper_digit, lower_digit, num;
            upper_digit = src[i + 1];
            lower_digit = src[i + 2];
            char hex_num[3] = {upper_digit, lower_digit, '\0'};
            num = (int) strtol(hex_num, NULL, 16);
            dst[j++] = num;
            i += 2;
        }
        else {
            dst[j++] = src[i];
        }
    }
    if (src[i - 1] != '%') {
        dst[j++] = src[i++];
        dst[j++] = src[i];
        dst[j] = '\0';
    }
}

char *check_file_extension(char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot || dot == path) return NULL;
    
    if (strcmp(dot, ".html") == 0) {
        return "text/html";
    }
    else if (strcmp(dot, ".ico") == 0) {
        return "image/x-icon";
    }
    else if (strcmp(dot, ".css") == 0) {
        return "text/css";
    }
    else if (strcmp(dot, ".jpg") == 0) {
        return "image/jpg";
    }
    else return NULL;
}

int check_ver(char *http_version) {
    if (strlen(http_version) > 5 && strncmp(http_version, "HTTP/", 5) == 0) {
        char *version = &http_version[5]; // Parse out version no

        // Convert to float
        float version_no = atof(version);
        printf("HTTP Version: %2f\n", version_no);

        if (version_no >= 0.9 && version_no < 4) return 1;
    }

    return 0;
}

FILE *check_resource(char *resource, char **full_path) {
    if (resource[0] != '/') return NULL;

    // Build the local path to the resource
    char *local_path;
    char base_path[PACK_LIM] = {'\0'};
    if (strcmp(resource, "/") == 0) { // Just get home
        local_path = "./www/index.html";
    }
    else {
        memcpy(base_path, "./www", 5);
        strlcat(base_path, resource, PACK_LIM - 6);
        local_path = base_path;        
    }

    // Percent decode the path
    char path_decoded[PACK_LIM] = {'\0'};
    url_decode(path_decoded, local_path);

    printf("Opening file path: %s\n", path_decoded);

    // Open the requested file
    FILE *asset = fopen(path_decoded, "r");
    if (asset == NULL) {
        return NULL;
    }
    
    // Return the decoded file path and file handle
    *full_path = strdup(path_decoded);
    return asset;
}

int parse_request(char *buffer, int *asset_fd, struct head *header_fields) {
    int treat_get = strncmp(buffer, "GET ", 4);
    if (treat_get != 0) {
        fprintf(stderr, "[ERROR]: Not a valid get request.\n");
        return 1;
    }

    // Tokenize resource and http version
    char *buffer_context; 
    strtok_r(buffer, " ", &buffer_context);
    char *resource = strtok_r(NULL, " ", &buffer_context); // Get the requested resource if any
    char *http_ver = strtok_r(NULL, " ", &buffer_context); // Get the HTTP Version
    if (!resource || !http_ver) {
        fprintf(stderr, "[ERROR]: Not a valid get request.\n");
        return 1;
    }

    // Check HTTP version
    char *ver_context;
    strtok_r(http_ver, "\n", &ver_context);
    if (!check_ver(http_ver)) {
        fprintf(stderr, "[ERROR]: Invalid HTTP version.\n");
        return 1;
    }

    // Check resource
    FILE *content_handle;
    char *file_path = NULL; // Obtain the full file path
    int not_found = 0, stack_path = 0;
    if ((content_handle = check_resource(resource, &file_path)) == NULL) {
        fprintf(stderr, "[ERROR]: Requested resource not found, returning 404.\n");

        // Set 404 parameters
        content_handle = fopen("./www/404.html", "r");
        if (content_handle == NULL) {
            fprintf(stderr, "[ERROR]: Could not open 404.\n");
            return 1;
        }
        file_path = strdup("./www/404.html");
        not_found = 1;
    }

    printf("Returning file path: %s\n", file_path);

    // Check and retrieve the file extension
    const char *extension = check_file_extension(file_path);
    free(file_path);
    if (extension == NULL) {
        fprintf(stderr, "[ERROR]: Unknown file extension.\n");
        return 1;
    }

    // Check the file size
    struct stat file_info;
    int fd = fileno(content_handle);
    if (fstat(fd, &file_info) == -1) {
        fprintf(stderr, "[ERROR]: Failed to stat file size.\n");
        return 1;
    }
    *asset_fd = fd; // Return the file descriptor

    // Build the fields for the packet (stream)
    header_fields->response = not_found ? "Not Found" : "Ok";
    header_fields->response_no = not_found ? 404 : 200;
    header_fields->content_type = extension;
    header_fields->content_length = file_info.st_size;

    return 0;
}

struct packet_node *response_list(char *buffer) {
    int fd; 
    struct head header;
    int failed_request = parse_request(buffer, &fd, &header);
    if (failed_request) return NULL;

    char *asset_bytes = mmap(NULL, header.content_length, PROT_READ , MAP_PRIVATE, fd, 0);
    if (asset_bytes == NULL) {
        fprintf(stderr, "[ERROR]: Fatal error mapping resource contents.\n");
        return NULL;
    }

    struct packet_node *head = (struct packet_node *) malloc(sizeof(struct packet_node));
    char packet_buffer[PACK_LIM] = {'\0'};
    int head_size = snprintf(packet_buffer, PACK_LIM - 1,  
        "HTTP/1.0 %d %s\r\n"
        "Content-Type: %s; charset=UTF-8\r\n"
        "Content-Length: %d\r\n"
        "Connection: Close\r\n\r\n", 
        header.response_no, header.response, header.content_type, header.content_length);

    // Create the first buffer node as the header
    head->length = head_size;
    head->packet = strdup(packet_buffer);
    head->next = NULL;
    
    // Read the contents into a series of packet buffers
    struct packet_node *curr = head;
    for (int i = 0; i < header.content_length; i += PACK_LIM - 1) {
        char buffered_send[PACK_LIM] = {'\0'};
        int length = fmin(PACK_LIM - 1, header.content_length - i);
        memcpy(buffered_send, asset_bytes + i, length);

        struct packet_node *node = (struct packet_node *) malloc(sizeof(struct packet_node));
        node->length = length;
        node->packet = (char *) calloc(length + 1, sizeof(char));
        memcpy(node->packet, buffered_send, length);
        node->next = NULL;

        // Add the node to the linked list
        curr->next = node;
        curr = node;
    }
    
    munmap(asset_bytes, header.content_length);

    return head;
    
}
