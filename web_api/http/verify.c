#include "verify.h"

int verify_ver(char *http_version) {
    for (int i = 0; i < 4; i++) {
        if (strcmp(http_version, HTTP_VERS[i]) == 0)
            return 1;
    }

    return 0;
}

int verify_req(char *request_type) {
    for (int i = 0; i < 9; i++) {
        if (strcmp(request_type, REQ_TYPES[i]) == 0) {
            return (req_type_mapping[i] == GET);
        }
    }

    return -1;
}


char *grab_extension(char *path) {
    if (!path) return NULL;

    const char *dot = strrchr(path, '.');
    if (!dot || dot == path)
        return NULL;

    printf("Here is the extension we grabbed: %s\n", dot);

    for (int i = 0; i < sizeof(match_field) / sizeof(http_ext); i++) {
        if (strcmp(dot, match_field[i].file_ext) == 0)
            return match_field[i].header_field;
    }

    return NULL;
}
