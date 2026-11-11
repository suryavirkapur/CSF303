#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>

void die(char *s, bool kill) {
    printf("[SERVER] [ERROR]");
    printf("%s\n", s);
    if (kill) exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) die("Could not initialize socket", 1);
    
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9091);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int bindRes = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (bindRes < 0) die("Could not bind", 1);

    int listenRes = listen(sockfd, 5);
    if (listenRes < 0) die("Could not listen", 1);

    while (1) {
        printf("[SERVER] Waiting for connnections\n");
        int clientfd = accept(sockfd, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
        if (clientfd < 0) {die("Couldn't make connection", 0);continue;}

        printf("[SERVER] Client Connected: %d", ntohs(clnt_addr.sin_port));

        while (1) {
            char buffer[1024];
            
            int bytes = recv(clientfd, buffer, 1023, 0);

            if (bytes < 0) {printf("[SERVER] Could not recieve"); break;}
            if (bytes == 0) {printf("[SERVER] Client Disconnectd: %d", ntohs(clnt_addr.sin_port));break;}
            buffer[bytes] = '\0';
            printf("[SERVER] Received: %s\n", buffer);
        }

        close(clientfd);
    }
    close(sockfd);
    return 0;    
}

