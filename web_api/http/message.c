#include "message.h"
#include "http.h"

void *message(void *header) {
    Header *data = (Header *) header;
    Message_Node *message = NULL;

    if (!header) 
        return NULL;
    if (!(message = (Message_Node *) malloc(sizeof(Message_Node))))
        return NULL;

    // Zero the response buffer
    memset(message->header, 0, sizeof(message->header));
    memset(message->body, 0, MTU);
    message->next = NULL;

    if (snprintf(message->header, MTU - 1,  
        "HTTP/1.0 %d %s\r\n"
        "Content-Type: %s; charset=UTF-8\r\n"
        "Content-Length: %d\r\n"
        "Connection: Close\r\n\r\n", 
        data->data.response_no, data->response_type,
        data->content_type, data->data.length) < 0)
    {
        return NULL;
    }
    
    if (fread(message->body, sizeof(char), MTU - 1, data->data.handle) == 0)
        return NULL;

    return message;
}

void *chunked_message(void *header) {
    Header *header_contents = (Header *) header;
    Message_Node *message = NULL;

    return message;
}

void *internal_error_message(void *header) {
    Header *header_contents = (Header *) header;
    Message_Node *message = NULL;

    return message;
}