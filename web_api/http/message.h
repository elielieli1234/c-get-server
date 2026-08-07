#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Return a single node message in the case that the
 *        response fits within the MTU.
 * 
 * @param header The header metadata structure
 * @return The head of the list as an opaque pointer,
 *         or NULL on failure
 */
void *message(void *header);

/**
 * @brief Return a linked list of chunked body responses
 *        in the case that the response does not fit within
 *        the MTU
 * 
 * @param header The header metadata structure
 * @return The head of the list as an opaque pointer,
 *         or NULL on failure
 */
void *chunked_message(void *header);

/**
 * @brief Return an internal error message
 * @param header The header metadata structure
 */
void *internal_error_message(void *header);

#endif