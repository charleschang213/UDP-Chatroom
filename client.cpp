#include "utils.h"

static volatile int serverSocket;
static pthread_t threads[2];

struct sockaddr_in addr;
struct hostent *serv;
unsigned int addrlength;

void *write_to_server(void *arg);
void *read_from_server(void *arg);
void close_program(int signal);

/**
 * Clean up for client
 * Called by close_program upon SIGINT
 */
void close_client()
{
    pthread_cancel(threads[0]);
    pthread_cancel(threads[1]);
}

/**
 * Close the program when upon failure
 * Called on the failure of initialization 
 */
void exit_failure()
{
    exit(1);
}

/**
 * Sets up a connection to a chatroom server and begins
 * reading and writing to the server.
 *
 * host     - Server to connect to.
 * port     - Port to connect to server on.
 * username - Name this user has chosen to connect to the chatroom with.
 */
void run_client(const char *host, const char *port, const char *username)
{
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)atoi(port));
    addrlength = sizeof(addr);
    serv = gethostbyname(host);

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    int optval = 1;
    if (serverSocket == -1)
    {
        perror(NULL);
        exit_failure();
    }
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    memcpy(&addr.sin_addr.s_addr, serv->h_addr, serv->h_length);

    struct timeval tv;
    tv.tv_sec = SOCKET_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    pthread_create(threads + 0, NULL, &write_to_server, (void *)username);
    pthread_create(threads + 1, NULL, &read_from_server, NULL);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    close(serverSocket);
}

typedef struct _thread_cancel_args
{
    char **buffer;
    char *name;
} thread_cancel_args;

/**
 * Cleanup routine in case the thread gets cancelled
 * Ensure buffers are freed if they point to valid memory
 */
void thread_cancellation_handler(void *arg)
{
    thread_cancel_args *a = (thread_cancel_args *)arg;
    char **buffer = a->buffer;
    char *name = a->name;
    if (*buffer)
    {
        delete []*buffer;
        *buffer = NULL;
    }

    if (name)
    {
        char temp_message[MSG_SIZE + HEADER_LENGTH] = {'\0'};
        strcpy(temp_message, QUIT_HEADER);
        sprintf(temp_message + HEADER_LENGTH, "%s left the chat!", name);
        sendto(serverSocket, temp_message, strlen(temp_message), 0, (struct sockaddr *)&addr, sizeof(addr));
    }
}

/**
 * Reads bytes from user and writes them to server
 *
 * arg - void* casting of char* that is the username of client
 */
void *write_to_server(void *arg)
{
    (void)arg;
    char *name = (char *)arg;
    char *buffer = new char[MSG_SIZE];
    ssize_t retval = 1;

    thread_cancel_args cancel_args;
    cancel_args.buffer = &buffer;
    cancel_args.name = name;
    pthread_cleanup_push(thread_cancellation_handler, &cancel_args);

    char temp_message[MSG_SIZE + HEADER_LENGTH] = {'\0'};
    strcpy(temp_message, JOIN_HEADER);
    sprintf(temp_message + HEADER_LENGTH, "%s joins the chat!", name);
    retval = sendto(serverSocket, temp_message, strlen(temp_message), 0, (struct sockaddr *)&addr, sizeof(addr));

    while (retval > 0)
    {
        std::cin.getline(buffer, MSG_SIZE);
        if (buffer == NULL)
            break;
        strcpy(temp_message, MSG_HEADER);
        sprintf(temp_message + HEADER_LENGTH, "%s : %s", name, buffer);
        retval = sendto(serverSocket, temp_message, strlen(temp_message), 0, (struct sockaddr *)&addr, sizeof(addr));
    }
    pthread_cleanup_pop(0);
    return 0;
}

/**
 * Reads bytes from the server and prints them to the user.
 *
 * arg - void* requriment for pthread_create function
 */
void *read_from_server(void *arg)
{
    (void)arg;
    ssize_t retval = 1;
    char *buffer = NULL;
    thread_cancel_args cancellation_args;
    cancellation_args.buffer = &buffer;
    cancellation_args.name = NULL;
    pthread_cleanup_push(thread_cancellation_handler, &cancellation_args);

    while (retval > 0)
    {
        buffer = new char[MSG_SIZE];
        ssize_t byte_count = recvfrom(serverSocket, buffer, MSG_SIZE, 0, (struct sockaddr *)&addr, &addrlength);
        buffer[byte_count] = 0;
        if (byte_count > 0)
        {
            switch (buffer[0])
            {
            case 'J':
                std::cout << GREEN_CODE;
                break;
            case 'M':
                std::cout << BLUE_CODE;
                break;
            case 'Q':
                std::cout << YELLOW_CODE;
                break;
            default:
                std::cout << RESET_CODE;
            }
            std::cout << buffer + HEADER_LENGTH << RESET_CODE << std::endl;
        }
        delete[] buffer;
        buffer = NULL;
    }

    pthread_cleanup_pop(0);
    return 0;
}

/**
 * Signal handler used to close this client program.
 */
void close_program(int signal)
{
    if (signal == SIGINT)
    {
        close_client();
    }
}

int main(int argc, char **argv)
{

    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, "Usage: %s <address> <port> <username>\n",
                argv[0]);
        exit(1);
    }

    signal(SIGINT, close_program);

    run_client(argv[1], argv[2], argv[3]);

    return 0;
}