#include "utils.h"

void *listen_from_clients(void *arg);
void write_clients(const char *message, size_t size);

static volatile int sessionEnd;
static volatile int serverSocket;

static volatile int clientsCount;
static volatile sockaddr_in clientList[MAX_CLIENTS];
static volatile int clientLength[MAX_CLIENTS];
static volatile int clientSlot[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct sockaddr_in serveraddr;
int serveraddr_length;

void close_server(int signal)
{
    if (signal == SIGINT)
    {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = -1;
        sessionEnd = 1;
    }
}

/**
 * Close the program upon failure
 * call close_server() to shut down the server socket
 */
void exit_failure()
{
    close_server(SIGINT);
    exit(1);
}

void run_server(char *port)
{

    int retval;

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(port));
    serveraddr_length = sizeof(serveraddr);

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1)
    {
        perror("socket():");
        exit_failure();
    }

    int optval = 1;
    retval = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (retval == -1)
    {
        perror("setsockopt():");
        exit_failure();
    }

    if (bind(serverSocket, (struct sockaddr *)&serveraddr, serveraddr_length))
    {
        perror("bind():");
        exit_failure();
    }

    std::cout << "Listening on file descriptor " << serverSocket << ", port " << ntohs(serveraddr.sin_port) << std::endl;
    std::cout << "Waiting for connection..." << std::endl;

    clientsCount = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clientSlot[i] = -1;
    }
    pthread_t thread;
    retval = pthread_create(&thread, NULL, listen_from_clients, NULL);
    if (retval != 0)
    {
        perror("pthread_create():");
        exit(1);
    }
    pthread_join(thread, NULL);
    if (serverSocket != -1)
    {
        close_server(SIGINT);
    }
}

void *listen_from_clients(void *arg)
{
    (void)arg;
    ssize_t retval = 1;
    while (retval > 0 && sessionEnd == 0)
    {
        struct sockaddr_in clientaddr;
        unsigned int clientaddr_length = sizeof(clientaddr);
        int recv_length = 0;
        char serverMessage[MSG_SIZE] = {'\0'};
        recv_length = recvfrom(serverSocket, serverMessage, sizeof(serverMessage), 0, (struct sockaddr *)&clientaddr, &clientaddr_length);
        if (sessionEnd == 0)
        {
            std::cout << "Receive Message(" << recv_length << " bytes): \"" << serverMessage << "\"" << std::endl;
            switch (serverMessage[0])
            {
            case 'J':
                pthread_mutex_lock(&mutex);
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clientSlot[i] == -1)
                    {
                        clientsCount++;
                        std::cout << "Client " << i << " joins" << std::endl;
                        memcpy((void *)&(clientList[i]), &clientaddr, sizeof(clientaddr));
                        clientLength[i] = clientaddr_length;
                        clientSlot[i] = 1;
                        sprintf(serverMessage + strlen(serverMessage), " Currently serving %d client(s).", clientsCount);
                        break;
                    }
                }
                pthread_mutex_unlock(&mutex);
                break;
            case 'Q':
                pthread_mutex_lock(&mutex);
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clientList[i].sin_addr.s_addr == clientaddr.sin_addr.s_addr && clientList[i].sin_port == clientaddr.sin_port)
                    {
                        std::cout << "Client " << i << " left" << std::endl;
                        clientSlot[i] = -1;
                        clientsCount--;
                        sprintf(serverMessage + strlen(serverMessage), " Currently serving %d client(s).", clientsCount);
                    }
                }
                pthread_mutex_unlock(&mutex);
                break;
            default:
                break;
            }
            write_clients(serverMessage, strlen(serverMessage));
        }
    }
    return NULL;
}

void write_clients(const char *message, size_t size)
{
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientSlot[i] != -1)
        {
            int send_length = sendto(serverSocket, message, size, 0, (struct sockaddr *)&(clientList[i]), clientLength[i]);
            if (send_length < 0)
            {
                perror("sendto() error");
                exit(1);
            }
            std::cout << "Sent " << send_length << " bytes to client " << i << std::endl;
        }
    }
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        std::cerr << "./server <port>" << std::endl;
        return -1;
    }

    signal(SIGINT, close_server);
    run_server(argv[1]);

    return 0;
}