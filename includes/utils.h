#pragma once

#include <string>
#include <cstdio>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>

/**
 * The largest size the message can be that a client
 * sends to the server.
 */

#define MAX_CLIENTS 8
#define MSG_SIZE (256)
#define SOCKET_TIMEOUT 2000

#define JOIN_HEADER "JOIN:"
#define MSG_HEADER "MESG:"
#define QUIT_HEADER "QUIT:"
#define HEADER_LENGTH 5

#define GREEN_CODE "\033[32m"
#define BLUE_CODE "\033[34m"
#define YELLOW_CODE "\033[33m"
#define RESET_CODE "\033[0m"