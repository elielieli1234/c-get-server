#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef const struct http_ext {
    char *file_ext;
    char *header_field;
} http_ext;

const http_ext match_field[4] = {
    {
        .file_ext     = ".html",
        .header_field = "text/html"
    },
    {
        .file_ext     = ".ico",
        .header_field = "image/x_icon"
    },
    {
        .file_ext     = ".css",
        .header_field = "text/css"
    },
    {
        .file_ext     =  ".jpg",
        .header_field = "image/jpg" 
    }
};

// 
const char *HTTP_VERS[] = {
    "HTTP/0.9",
    "HTTP/1.0",
    "HTTP/1.1",
    "HTTP/2",
    "HTTP/3"
};

const char *REQ_TYPES[] = {
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "CONNECT",
    "OPTIONS",
    "TRACE",
    "PATCH"
};

typedef enum REQ_TYPES_E { 
    GET, 
    HEAD, 
    POST, 
    PUT, 
    DELETE, 
    CONNECT, 
    OPTIONS, 
    TRACE, 
    PATCH
} REQ_TYPES_E;

const REQ_TYPES_E req_type_mapping[] = {
    GET, 
    HEAD, 
    POST, 
    PUT, 
    DELETE, 
    CONNECT, 
    OPTIONS, 
    TRACE, 
    PATCH 
};

/**
 * @brief Verify the http version matches some known version
 * @param  http_version The http version as a string
 * @return 1 if it matches, 0 if it doesn't match any
 */
int verify_ver(char *http_version);

/**
 * @brief Verify the request type matches some known version
 * @param  request_type The request type as a string
 * 
 * @return -1 if it does not match a known request,
 *          0 if it is not a GET request, and 1 if it is.
 */
int verify_req(char *request_type);

/**
 * @brief Verify the file extension matches some known extension type
 * @param  path The file extension path 
 * @return The file extension as a handle to a global string
 */
char *grab_extension(char *path);

#endif