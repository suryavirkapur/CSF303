#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>

void error(char *s){
    printf("[ERROR]"); printf(s); exit(1);
}

int main(int argc, char * argv[]) {
    int clntfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clntfd < 0) error("ERROR");

    struct sockaddr_in srvr_addr;
    srvr_addr.sin_family = AF_INET;
    srvr_addr.sin_port = htons(9090);
    srvr_addr.sin_addr.s_addr = INADDR_LOOPBACK;

    int connectErr = connect(clntfd, (struct sockaddr*)&srvr_addr, sizeof(srvr_addr));
    if (connectErr < 0) error("Connection Error");

    printf("Connected to Server\n");

    while(1){

    }
    
    return 0;
}