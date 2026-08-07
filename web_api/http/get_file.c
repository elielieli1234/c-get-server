#include "get_file.h"
#include "response.h"
#include "http.h"

static const char *root            = "./www";
static const char *home            = "/index.html";
static const char *bad_request     = "/400.html";
static const char *not_found       = "/404.html";
static const char *not_permitted   = "/405.html";
static const char *not_implemented = "/501.html";
static const char *server_busy     = "/503.html";

static void sanitize_traversal(char *path, int length, int *response) {
    for (int i = 0; i < length - 1; i++) {
        if (path[i] == '.' && path[i + 1] == '.') {
            *response = NOT_FOUND; return;
        } 
    } 
}

static void url_decode(char *dst, char *src, int *length) {
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

    *length = j;
}

void write_err_endpoint(char *path, int path_len, int response_no) {
    memset(path, 0, path_len);
    snprintf(path, path_len - 1, "%s/err/%d.html", root, response_no);
}

void file_try_open(char *resource, void *header_metadata, int response) {
    int response_no  = (resource && *resource == '/') ? response : BAD_REQUEST;
    int expanded_len = 0, exists = 0, fd = 0;
    
    char base_path[MTU]     = {0};
    char expanded_path[MTU] = {0};
    char *final_path = NULL;

    Header_Metadata *data = (Header_Metadata *) header_metadata;
    FILE *asset_handle = NULL;

    // Write the requested resource or home path
    if (response_no == OK) {
        memcpy(base_path, root, strlen(root));
        if (strcmp(resource, "/") == 0) strncat(base_path, home, strlen(home));
        else strncat(base_path, resource, MTU - strlen(base_path) - 1);
        url_decode(expanded_path, base_path, &expanded_len);
        sanitize_traversal(expanded_path, expanded_len, &response_no);
    }

    if (response_no != OK)
        write_err_endpoint(expanded_path, sizeof(expanded_path), response_no);

    // Modify the error code & uri as necessary
    struct stat path_stat = {0};
    exists = stat((const char *) expanded_path, &path_stat);
    if (exists < 0) {
        if (errno == ENOENT) response_no = NOT_FOUND;
        else if (errno == EACCES) response_no = FORBIDDEN;
        else goto internal_error;
        write_err_endpoint(expanded_path, sizeof(expanded_path), response_no);
    }

    if (!(final_path = strdup(expanded_path)))    goto internal_error;
    if (!(asset_handle = fopen(final_path, "r"))) goto internal_error;
      
    data->handle = asset_handle;
    data->uri = final_path;
    data->response_no = response_no;
    data->length = path_stat.st_size;
    data->fd = fileno(asset_handle); // Per man should not fail
    data->chunk = (path_stat.st_size > MTU - 1);

    return;

internal_error:
    if (final_path)  free(final_path);
    if (asset_handle) fclose(asset_handle);
    
    data->handle = NULL;
    data->uri    = NULL;
    data->response_no = INTERNAL_ERROR;
    data->length = 0;
    data->fd     = 0;
}




