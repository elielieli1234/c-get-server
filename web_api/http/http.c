#include "http.h"
#include "response.h"

static const char *response_strings[] = {
    "Bad Request",
    "Forbidden",
    "Not Found",
    "Not Implemented",
    "Server Busy",
    "Internal Server Error",
    "Ok"
};

static const int response_ints[] = {
    BAD_REQUEST, 
    FORBIDDEN, 
    NOT_FOUND, 
    NOT_IMPLEMENTED, 
    SERVER_BUSY, 
    INTERNAL_ERROR, 
    OK
};

// Request parsing
extern int  verify_req(char *request_type);
extern int  verify_ver(char *http_version);
extern char *grab_extension(char *path);

// Graceful file open
extern void file_try_open(char *resource, void *header_metadata, int response_no);

// Message construction
extern void *message(void *header);
extern void *chunked_message(void *header);
extern void *internal_error_message(void *header);

static void print_header(Header *response);

/**
 * @brief Parse the semantic tokens of the HTTP request
 * @param buffer The contents of the request as a byte array
 * @param header A pointer to write back the response metadata
 */
void request_parse(char *buffer, Header *header) { 
    const char *file_extension, *response_type;
    char *line_context, *field_context; 

    // Hold the semantic fields of the request
    char *request_type  = NULL;
    char *resource_path = NULL;
    char *http_version  = NULL;

    const int response_map_len = sizeof(response_ints) / sizeof(int);
    int response = OK, get = -1;

    Header_Metadata data = {0};
    FILE *asset_handle    = NULL;
    
    // Grab the first line
    char *line = strtok_r(buffer, "\n\r", &line_context);
    if (!line) {
        response = BAD_REQUEST;
        goto construct_msg;
    }

    // Grab the constituent fields
    if (!(request_type  = strtok_r(line, " ", &field_context)) ||
        !(resource_path = strtok_r(NULL, " ", &field_context)) ||
        !(http_version  = strtok_r(NULL, " ", &field_context)))
    {
        response = BAD_REQUEST;
        goto construct_msg;
    }

    // Verify the request type
    get = verify_req(request_type);
    if (get != 1) {
        if (get == 0)  response = NOT_IMPLEMENTED;
        if (get == -1) response = BAD_REQUEST; // TODO FIX 400 (wrong)
        goto construct_msg;
    }

    // Verify the HTTP version and downgrade
    if (!verify_ver(http_version)) {
        response = BAD_REQUEST;
        goto construct_msg;
    }

construct_msg: // Fill the remaining header fields
    file_try_open(resource_path, (void *) &data, response);
    if (data.uri) file_extension = grab_extension(data.uri);
    for (int i = 0, response = data.response_no; i < response_map_len; i++) {
        if (response == response_ints[i]) {
            response_type = response_strings[i];
            break;
        }
    }

    header->response_type = response_type;
    header->content_type  = file_extension;
    header->data = data;
}

struct message_node *response_list(char *buffer) {
    Message_Node *head = NULL;
    Header response = {0}; // Verify the request and grab metadata

    request_parse(buffer, &response);
    print_header(&response);

    // Concatenate the response header and body
    if (response.data.response_no == INTERNAL_ERROR) 
        head = internal_error_message((void *) &response);
    else if (response.data.chunk)
        head = chunked_message((void *) &response);
    else 
        head = message((void *) &response);

    if (!head) {
        if (response.data.handle) { 
            fclose(response.data.handle);
            response.data.handle = NULL;
        }
        if (response.data.uri) {
            free(response.data.uri);
            response.data.uri = NULL;
        }
        while (head) {
            Message_Node *next = head->next;
            free(head);
            head = next;
        }

        return NULL;
    }

    return head;
}

static void print_header(Header *response) {
    printf("I think we got a response, let's take a look...\n");
    printf("Content type  : %s\n", response->content_type  ? (response->content_type)  : "NA");
    printf("Response type : %s\n", response->response_type ? (response->response_type) : "NA");
    printf("Metadata structure ~ \n");
    printf("    File handle     : %p\n", response->data.handle ? response->data.handle : 0);
    printf("    File path       : %s\n", response->data.uri ? response->data.uri : "NA");
    printf("    Response number : %d\n", response->data.response_no);
    printf("    Response length : %d\n", response->data.length);
    printf("    Response fd     : %d\n", response->data.fd);
}