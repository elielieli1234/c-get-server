#ifndef GET_FILE_H
#define GET_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

/**
 * @brief Build the uri and open the path to a resource
 * @param resource       The requested resource as a string
 * @param header         A pointer to write back the header structure      
 * @param response       The current response number
 * @return A header metadata structure as an opaque type
 */
void file_try_open(char *resource, void *header_metadata, int response);

#endif