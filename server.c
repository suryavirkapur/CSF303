#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXPENDING 5
#define BUFFERSIZE 32
#define MAX_CONNECTIONS 3

void handleClient(int clientSocket, struct sockaddr_in clientAddress)
{
    char msg[BUFFERSIZE];

    int temp2 = recv(clientSocket, msg, BUFFERSIZE, 0);
    if (temp2 < 0)
    {
        printf("Error in receiving message\n");
        exit(1);
    }
    double x = atof(msg) + 1;
    int y = x;
    sprintf(msg, "%d", y);
    printf("Received from client: %s\n", msg);
    int bytesSent = send(clientSocket, msg, strlen(msg), 0);
    if (bytesSent != strlen(msg))
    {
        printf("Error while sending message to client\n");
        exit(1);
    }

    close(clientSocket);
    exit(0);
}

int main()
{
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0)
    {
        printf("Error while server socket creation\n");
        exit(1);
    }
    printf("Server Socket Created\n");

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(7779);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        printf("Error in setting socket options\n");
        exit(1);
    }

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error while binding\n");
        exit(1);
    }
    printf("Binding successful\n");

    if (listen(serverSocket, MAXPENDING) < 0)
    {
        printf("Error in listen\n");
        exit(1);
    }
    printf("Now Listening\n");

    int activeConnections = 0;

    while (1)
    {
        struct sockaddr_in clientAddress;
        int clientLength = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLength);
        if (clientSocket < 0)
        {
            printf("Error in accepting client\n");
            continue;
        }

        if (activeConnections >= MAX_CONNECTIONS)
        {
            printf("Reached max connections.\n");
            close(clientSocket);
            continue;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            printf("Error in fork\n");
            close(clientSocket);
            continue;
        }

        if (pid == 0)
        {
            close(serverSocket);
            handleClient(clientSocket, clientAddress);
        }
        else
        {
            close(clientSocket);
            activeConnections++;

            // This is what we had too do
            while (waitpid(-1, NULL, WNOHANG) > 0)
            {
                activeConnections--;
            }
        }
    }

    close(serverSocket);
    return 0;
}