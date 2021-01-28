#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "includes/utils.h"

static const size_t MESSAGE_SIZE_DIGITS = 4;

chatroom_message *create_message(std::string name, std::string message, int status) {
    chatroom_message *msg = new chatroom_message;
    msg->name = name;
    msg->message = message;
    msg->status = status;
    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    ssize_t bytes_read = 0;
    ssize_t bytes_remaining = count;
    ssize_t retval;

    while (bytes_remaining > 0 || (retval == -1 && errno == EINTR)) {
        retval = read(socket, (void *) (buffer + bytes_read), bytes_remaining);
        if (retval == 0)
            return 0;
        if (retval > 0) {
            bytes_read      += retval;
            bytes_remaining -= retval;
        }

    }
    return count;
}

ssize_t write_message_size(size_t size, int socket) {
    int32_t message_size = htonl(size);

    ssize_t write_bytes =
        write_all_to_socket(socket, (char *)&message_size, MESSAGE_SIZE_DIGITS);

    return write_bytes;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    ssize_t bytes_read = 0;
    ssize_t bytes_remaining = count;
    ssize_t retval;

    while (bytes_remaining > 0 || (retval == -1 && errno == EINTR)) {
        retval = write(socket, (void *) (buffer + bytes_read), bytes_remaining);
        if (retval == 0)
            return 0;
        if (retval > 0) {
            bytes_read      += retval;
            bytes_remaining -= retval;
        }
    }
    return count;
}